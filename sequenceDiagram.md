# UCI Engine Sequence Diagram

Sequence diagram for the chess engine using UCI protocol.  
**Team: The Broncos Gambit - Group 6**

```mermaid
sequenceDiagram
    autonumber
    participant Main as Main loop
    participant UCI as UCI parser
    participant Pos as Position (Board)
    participant Gen as Move generator
    participant Rules as Rule checker
    participant M as Move

    %% --- Handshake: Arena -> Engine ---
    Main->>UCI: readLine()
    UCI-->>Main: cmd="uci"
    Main-->>UCI: send("id name MyEngine")
    Main-->>UCI: send("id author TeamX")
    Main-->>UCI: send("uciok")

    %% --- Readiness check ---
    Main->>UCI: readLine()
    UCI-->>Main: cmd="isready"
    Main-->>UCI: send("readyok")