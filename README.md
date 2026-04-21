# The-Bronco's-Gambit-Group-6-Repository
Repository for ECE 4318 files

## Engine Features
* **Negamax Search** (A streamlined variant of the minimax algorithm that calculates the best sequence of moves by maximizing the score from the active player's perspective.)
* **Iterative Deepening** (Repeatedly searches the position at increasing depths to ensure a strong, calculated move is always available if the time limit is abruptly reached.)
* **Alpha-Beta Pruning** (Drastically reduces the number of board states evaluated by skipping branches that are mathematically proven to be worse than previously found moves.)
* **Piece-Square Tables (PSTs)** (Assigns positional bonuses or penalties to pieces based on where they are located on the board to encourage active development and center control.)
* **Tapered Evaluation** (Smoothly transitions the evaluation weights of pieces and squares between the middlegame and endgame based on the amount of non-pawn material left on the board.)
* **Pawn Structure Analysis** (Evaluates the pawn skeleton by tracking pawn distribution across files. It applies structural penalties for doubled pawns (multiple friendly pawns on the same file) and isolated pawns (pawns with no adjacent friendly defenders). Conversely, it assigns scaling bonuses to passed pawns based on how close they are to promotion.)
* **King Safety Evaluation** (Assesses the vulnerability of the king during the middlegame. It checks for intact protective pawn shields directly in front of the king and applies severe penalties if the king is left exposed on or adjacent to fully open files where enemy heavy pieces could easily attack.)

## Performance

Based on a 1000-game match against Stockfish at UCI Skill Level 1, the engine's performance can be estimated.

*   **Opponent:** Stockfish @ Skill Level 1 (Estimated ~1350 Elo).
*   **Result:** -63.6 +/- 20.9 Elo difference.
*   **Estimated Elo:** ~1290

The engine performs significantly better as White ([0.513] score) than as Black ([0.306] score), indicating the first-move advantage is a factor in its performance.

## Benchmarking

To replicate the performance test, you can use `cutechess-cli`. Ensure both `cutechess-cli` and `stockfish` are installed and accessible in your system's PATH. The engine must be compiled first (e.g., by running `make all`).

You can run the benchmark using the provided shell script, which contains the full command:
```shell
./cutechess-vs_stockfish.sh
```

This script runs a 1000-game tournament (500 rounds, 2 games per round) with a time control of 10 seconds + 0.08s increment per move. The results will be saved to `tests/tournament.pgn`.