/* CLIENT.C - Task 6 -------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 6
 Author      : Candidate 102
 Description : Network client for the EWA TCP over TCP protocol
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "include/ewpdef.h"
#include "include/client.h"


/*=== Constants: ========================================================================
=======================================================================================*/

#define FLAG_FIN         0x01   
#define FLAG_ACK         0x10   
#define FLAG_NACK        0x20   

#define MAX_NACK_RETRIES 2      
#define MAX_OOO_SLOTS    64     
#define MAX_PACKET_SIZE  131072u

#define CHECKSUM_OFFSET  16     


/*=== CRC16 IBM checksum: ==============================================================
=======================================================================================*/

static unsigned short crc16(const unsigned char *data, unsigned int len) {
    unsigned short crc = 0x0000;
    unsigned int i;
    int b;

    for(i = 0; i < len; i++) {
        crc ^= (unsigned short)data[i];
        for(b = 0; b < 8; b++) {
            /* 0xA001 is the bit reversed form of the CRC-16 polynomial 0x8005 */
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001u : (crc >> 1);
        }
    }
    return crc;
}

static unsigned short calc_packet_crc(unsigned char *buf, unsigned int len) {
    unsigned short result;
    unsigned char  s0 = buf[CHECKSUM_OFFSET];
    unsigned char  s1 = buf[CHECKSUM_OFFSET + 1];

    /* The sender zeroed these bytes before computing the checksum, so we do the same
       before recalculating, then restore the original bytes afterwards */

    buf[CHECKSUM_OFFSET]     = 0;
    buf[CHECKSUM_OFFSET + 1] = 0;
    result = crc16(buf, len);
    buf[CHECKSUM_OFFSET]     = s0;
    buf[CHECKSUM_OFFSET + 1] = s1;
    return result;
}


/*=== Socket helpers: ===================================================================
=======================================================================================*/

/* Keep reading until we have exactly len bytes, because recv can return less
   than requested even when there is more data coming */
static int recv_all(int sock, unsigned char *buf, unsigned int len) {
    unsigned int done = 0;
    int r;

    while(done < len) {
        r = (int)recv(sock, buf + done, len - done, 0);
        if(r <= 0) {
            if(r == 0) fprintf(stderr, "[INFO] Server closed the connection\n");
            else       perror("recv");
            return -1;
        }
        done += (unsigned int)r;
    }
    return 0;
}

/* Same idea for sending, send can also return before all bytes are written */
static int send_all(int sock, const unsigned char *buf, unsigned int len) {
    unsigned int done = 0;
    int s;

    while(done < len) {
        s = (int)send(sock, buf + done, len - done, 0);
        if(s <= 0) { perror("send"); return -1; }
        done += (unsigned int)s;
    }
    return 0;
}


/*=== Build response: ===================================================================
=======================================================================================*/

/* Builds an ACK or NACK packet to send back to the server. We echo the sequence
   number in the ack field so the server knows which packet we are responding to */

static unsigned int build_response(unsigned char *buf, unsigned int buf_size,
                                   unsigned int ack_seq,
                                   unsigned short src_port,
                                   unsigned short dst_port,
                                   unsigned char flags) {
    struct EWA_EXAM25_TASK4_PROTOCOL_TCP *p;
    unsigned int total = (unsigned int)sizeof(struct EWA_EXAM25_TASK4_PROTOCOL_TCP) - 1u;

    if(buf_size < total) return 0;
    memset(buf, 0, total);

    p = (struct EWA_EXAM25_TASK4_PROTOCOL_TCP *)buf;
    p->usSourcePort      = src_port;
    p->usDestinationPort = dst_port;
    p->uiSequenceNumber  = 0;
    p->uiAckNumber       = ack_seq;
    p->ucDataOffset      = 5;
    p->ucReserved        = 0;
    p->ucFlags           = flags;
    p->usSizeOfPacket    = 0;
    p->usChecksum        = 0;
    p->usUnused          = 0;

    /* Compute and set the checksum before sending */
    p->usChecksum = calc_packet_crc(buf, total);
    return total;
}


/*=== Out of order buffer: ==============================================================
=======================================================================================*/

/* Each slot holds one packet worth of payload that arrived before we were ready for it.
   When data is NULL but valid is 1, the slot is being used as a NACK retry counter
   instead, and data_len holds how many NACKs we have sent for that sequence number */

typedef struct {
    unsigned int   seq;
    unsigned char *data;
    unsigned int   data_len;
    int            is_fin;
    int            valid;
} OooSlot;

/* Find a slot by sequence number, returns the index or -1 if not found */
static int ooo_find(OooSlot *s, unsigned int seq) {
    int i;
    for(i = 0; i < MAX_OOO_SLOTS; i++) {
        if(s[i].valid && s[i].seq == seq) return i;
    }
    return -1;
}

/* Store a packet payload in the first free slot */
static int ooo_store(OooSlot *s, unsigned int seq,
                     const unsigned char *data, unsigned int dlen, int is_fin) {
    int i;
    unsigned char *copy;

    /* Skip if we already have this sequence number stored */
    if(ooo_find(s, seq) >= 0) return 0;

    for(i = 0; i < MAX_OOO_SLOTS; i++) {
        if(!s[i].valid) {
            copy = (unsigned char *)malloc(dlen > 0u ? dlen : 1u);
            if(!copy) return -1;
            if(dlen > 0) memcpy(copy, data, dlen);
            s[i].seq      = seq;
            s[i].data     = copy;
            s[i].data_len = dlen;
            s[i].is_fin   = is_fin;
            s[i].valid    = 1;
            return 0;
        }
    }

    fprintf(stderr, "[WARN] Out-of-order buffer is full, dropping packet\n");
    return -1;
}

/* Free all allocated payload buffers and reset every slot */
static void ooo_free_all(OooSlot *s) {
    int i;
    for(i = 0; i < MAX_OOO_SLOTS; i++) {
        if(s[i].valid && s[i].data) {
            free(s[i].data);
            s[i].data = NULL;
        }
        s[i].valid = 0;
    }
}


/*=== Growing file buffer: ==============================================================
=======================================================================================*/

/* Simple growable buffer that collects all received payload before writing to disk.
   Starts at 1 MB and doubles whenever it runs out of space */

typedef struct {
    unsigned char *buf;
    unsigned int   size;
    unsigned int   used;
} FileBuf;

static int fb_init(FileBuf *f) {
    f->buf  = (unsigned char *)malloc(1024u * 1024u);
    f->size = 1024u * 1024u;
    f->used = 0;
    return f->buf ? 0 : -1;
}

static int fb_append(FileBuf *f, const unsigned char *data, unsigned int len) {
    unsigned char *nb;
    unsigned int   ns;

    if(len == 0) return 0;

    /* Grow the buffer if the new data would not fit */
    if(f->used + len > f->size) {
        ns = f->size * 2u;
        if(ns < f->used + len) ns = f->used + len + 1024u * 1024u;
        nb = (unsigned char *)realloc(f->buf, ns);
        if(!nb) return -1;
        f->buf  = nb;
        f->size = ns;
    }

    memcpy(f->buf + f->used, data, len);
    f->used += len;
    return 0;
}

static void fb_free(FileBuf *f) {
    if(f->buf) {
        free(f->buf);
        f->buf = NULL;
    }
    f->size = f->used = 0;
}


/*=== Run client: =======================================================================
=======================================================================================*/

int run_client(const char *server_ip, int port, const char *output_file) {
    int    sock;
    struct sockaddr_in addr;
    unsigned char *raw;
    unsigned char  resp[128];
    struct EWA_EXAM25_TASK4_PROTOCOL_TCP *pkt;
    unsigned int hdr_size;
    unsigned int resp_size;
    unsigned int total_size;
    int done;
    int total_packets;
    int ooo_idx;
    OooSlot ooo[MAX_OOO_SLOTS];
    FileBuf fb;

    /* Allocate a receive buffer large enough for the biggest possible packet */
    raw = (unsigned char *)malloc(MAX_PACKET_SIZE);
    if(!raw) {
        fprintf(stderr, "[ERR] malloc failed for receive buffer\n");
        return CLIENT_ERROR;
    }

    /* The struct has a 1 byte Data field at the end, so the true header size
       is sizeof the struct minus that one byte */
    hdr_size = (unsigned int)sizeof(struct EWA_EXAM25_TASK4_PROTOCOL_TCP) - 1u;

    memset(ooo, 0, sizeof(ooo));

    if(fb_init(&fb) < 0) {
        free(raw);
        return CLIENT_ERROR;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0) {
        perror("socket");
        fb_free(&fb);
        free(raw);
        return CLIENT_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);
    addr.sin_addr.s_addr = inet_addr(server_ip);

    printf("*** PG3401 2026 - Task 6 - TCP Client ***\n");
    printf("Connecting to %s:%d\n", server_ip, port);

    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        fb_free(&fb);
        free(raw);
        return CLIENT_ERROR;
    }

    printf("[INFO] Connected to %s:%d\n", server_ip, port);

    done          = 0;
    total_packets = 0;

    while(!done) {
        unsigned short data_size;
        unsigned short got_crc;
        unsigned short calc_crc;
        unsigned int   seq;
        unsigned char  flags;
        int            is_fin;
        int            nack_count;

        /* Read the fixed size header first so we know how many payload bytes follow */
        if(recv_all(sock, raw, hdr_size) < 0) {
            fprintf(stderr, "[ERR] Failed reading packet header\n");
            break;
        }

        pkt       = (struct EWA_EXAM25_TASK4_PROTOCOL_TCP *)raw;
        seq       = pkt->uiSequenceNumber;
        data_size = pkt->usSizeOfPacket;
        flags     = pkt->ucFlags;
        is_fin    = (flags & FLAG_FIN) ? 1 : 0;

        printf("[PKT] Seq=%u Flags=0x%02X Size=%u\n", seq, flags, data_size);

        total_size = hdr_size + (unsigned int)data_size;

        if(data_size > 0) {
            if(total_size > MAX_PACKET_SIZE) {
                fprintf(stderr, "[ERR] Packet too large (%u bytes), sending NACK\n", total_size);
                resp_size = build_response(resp, sizeof(resp), seq,
                                           pkt->usDestinationPort, pkt->usSourcePort, FLAG_NACK);
                if(resp_size > 0) send_all(sock, resp, resp_size);
                continue;
            }

            /* Now read the payload bytes that follow directly after the header */
            if(recv_all(sock, raw + hdr_size, (unsigned int)data_size) < 0) {
                fprintf(stderr, "[ERR] Failed reading payload for seq=%u\n", seq);
                break;
            }
        }

        got_crc  = pkt->usChecksum;
        calc_crc = calc_packet_crc(raw, total_size);

        printf("[CRC] Got: 0x%04X  Calc: 0x%04X\n", got_crc, calc_crc);

        if(got_crc != calc_crc) {
            printf("[WARN] Checksum mismatch for seq=%u\n", seq);

            /* Check how many NACKs we have already sent for this sequence number */
            nack_count = 0;
            ooo_idx    = ooo_find(ooo, seq);
            if(ooo_idx >= 0 && ooo[ooo_idx].data == NULL) {
                nack_count = (int)ooo[ooo_idx].data_len;
            }

            if(nack_count < MAX_NACK_RETRIES) {
                printf("[NACK] Requesting resend for seq=%u\n", seq);
                resp_size = build_response(resp, sizeof(resp), seq,
                                           pkt->usDestinationPort, pkt->usSourcePort, FLAG_NACK);
                if(resp_size > 0) send_all(sock, resp, resp_size);

                /* Register a counter slot or increment the existing one */
                if(ooo_idx < 0) {
                    int i;
                    for(i = 0; i < MAX_OOO_SLOTS; i++) {
                        if(!ooo[i].valid) {
                            ooo[i].seq      = seq;
                            ooo[i].data     = NULL;
                            ooo[i].data_len = 1;
                            ooo[i].is_fin   = 0;
                            ooo[i].valid    = 1;
                            break;
                        }
                    }
                } else {
                    ooo[ooo_idx].data_len++;
                }
                continue;
            }

            /* We have retried enough times, just take the packet as it is */
            printf("[WARN] Max NACK retries reached for seq=%u, accepting anyway\n", seq);
            if(ooo_idx >= 0 && ooo[ooo_idx].data == NULL) {
                ooo[ooo_idx].valid = 0;
            }
        }

        /* Packet is good, store it in the out-of-order buffer */
        if(ooo_store(ooo, seq, raw + hdr_size, (unsigned int)data_size, is_fin) == 0) {
            total_packets++;
            printf("[STORE] Seq=%u Size=%u Total=%d\n", seq, data_size, total_packets);
        }

        /* Send ACK so the server knows we got this packet */
        printf("[ACK] Acknowledging seq=%u\n", seq);
        resp_size = build_response(resp, sizeof(resp), seq,
                                   pkt->usDestinationPort, pkt->usSourcePort, FLAG_ACK);
        if(resp_size > 0) send_all(sock, resp, resp_size);

        if(is_fin) {
            printf("[INFO] FIN received, transfer complete\n");
            done = 1;
        }
    }

    close(sock);

    if(done) {
        unsigned int total_bytes = 0;
        unsigned int next_seq    = 0;
        FILE *fp;

        /* Walk through the out of order buffer in sequence order, appending each
           chunk to the file buffer and freeing the slot as we go */
        do {
            ooo_idx = ooo_find(ooo, next_seq);
            if(ooo_idx >= 0 && ooo[ooo_idx].data != NULL) {
                if(fb_append(&fb, ooo[ooo_idx].data, ooo[ooo_idx].data_len) < 0) break;
                total_bytes        += ooo[ooo_idx].data_len;
                next_seq           += ooo[ooo_idx].data_len;
                free(ooo[ooo_idx].data);
                ooo[ooo_idx].data   = NULL;
                ooo[ooo_idx].valid  = 0;
            } else {
                break;
            }
        } while(1);

        printf("[INFO] Packets=%d  Bytes=%u\n", total_packets, total_bytes);

        fp = fopen(output_file, "wb");
        if(!fp) {
            perror("fopen");
            ooo_free_all(ooo);
            fb_free(&fb);
            free(raw);
            return CLIENT_ERROR;
        }

        /* Write the whole re assembled file in one shot */
        if(fwrite(fb.buf, 1, fb.used, fp) != fb.used) {
            fprintf(stderr, "[ERR] fwrite did not write all bytes\n");
            fclose(fp);
            ooo_free_all(ooo);
            fb_free(&fb);
            free(raw);
            return CLIENT_ERROR;
        }

        fclose(fp);
        printf("[INFO] Saved %u bytes to %s\n", fb.used, output_file);
    }

    /* Clean up everything */
    ooo_free_all(ooo);
    fb_free(&fb);
    free(raw);

    printf("*** Program finished. Goodbye! ***\n");
    return done ? CLIENT_OK : CLIENT_ERROR;
}

/*END OF FILE ----------------------------------------------------------------*/

