# 🖥️ C Programming Exam - PG3401 (Spring 2026)
### 🏆 Karakter: A

Dette repositoriet inneholder min besvarelse for hjemmeeksamen i C-programmering ved Høyskolen Kristiania. Oppgaven fokuserte på lavnivå systemprogrammering og implementering av protokoller i C-89.

---

## 🛠️ Teknisk fokus
Prosjektet er bygget rundt følgende kjerneområder:

*   **Systemprogrammering:** Bruk av Linux systemkall og POSIX-standarder.
*   **Fil- og minnehåndtering:** Manipulasjon av binære datafiler og dynamisk allokering.
*   **Multithreading:** Implementering av trådsikkerhet ved bruk av `mutex` for å håndtere race conditions i matematiske operasjoner.
*   **Nettverk:** Implementering av rå TCP-kommunikasjon basert på Socket API-et, samt utvikling av en egen protokoll ("TCP over TCP") for feilsikker filoverføring.

---

## 🧩 Utvalgte utfordringer
Besvarelsen krevde problemløsning knyttet til:

*   **Protokoll-implementering:** Utvikling av en klient-server arkitektur som håndterer dataoverføring med CRC-sjekk og ACK/NACK-logikk.
*   **Robusthet:** Implementering av feilhåndtering for å sikre at serveren forblir stabil ved mottak av uventet eller malformert input.
*   **Effektivitet:** Optimalisering av ressursbruk ved bruk av tråder for tidkrevende matematiske tester.

---

*Utviklet som en del av studiet i Cyber Security ved Høyskolen Kristiania.*