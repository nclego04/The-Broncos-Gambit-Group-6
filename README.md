# The-Bronco's-Gambit-Group-6-Repository
Repository for ECE 4318 files

## Engine Features
* **Negamax Search** (A streamlined variant of the minimax algorithm that calculates the best sequence of moves by maximizing the score from the active player's perspective.)
* **Iterative Deepening** (Repeatedly searches the position at increasing depths to ensure a strong, calculated move is always available if the time limit is abruptly reached.)
* **Alpha-Beta Pruning** (Drastically reduces the number of board states evaluated by skipping branches that are mathematically proven to be worse than previously found moves.)
* **Quiescence Search** (Mitigates the horizon effect by continuing the search past the depth limit for tactical moves like captures and promotions until a stable position is reached.)
* **MVV-LVA Move Ordering** (Sorts pseudo-legal moves to evaluate high-value captures by low-value pieces first, exponentially increasing the efficiency of alpha-beta pruning.)
* **Piece-Square Tables (PSTs)** (Assigns positional bonuses or penalties to pieces based on where they are located on the board to encourage active development and center control.)
* **Tapered Evaluation** (Smoothly transitions the evaluation weights of pieces and squares between the middlegame and endgame based on the amount of non-pawn material left on the board.)
* **Pawn Structure Analysis** (Evaluates the pawn skeleton by tracking pawn distribution across files. It applies structural penalties for doubled pawns (multiple friendly pawns on the same file) and isolated pawns (pawns with no adjacent friendly defenders). Conversely, it assigns scaling bonuses to passed pawns based on how close they are to promotion.)
* **King Safety Evaluation** (Assesses the vulnerability of the king during the middlegame. It checks for intact protective pawn shields directly in front of the king and applies severe penalties if the king is left exposed on or adjacent to fully open files where enemy heavy pieces could easily attack.)
