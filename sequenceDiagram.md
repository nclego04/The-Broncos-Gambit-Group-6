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