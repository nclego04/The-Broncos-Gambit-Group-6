def pseudo_legal_moves(p):
    moves = []
    us_white = p.white_to_move
    for i in range(64):
        pc = p.b[i]
        if pc == '.': continue
        white = is_white_piece(pc)
        if white != us_white: continue
        
        up = pc.upper()
        if up == 'P':
            moves.extend(gen_pawn(p, i, white))
        elif up == 'N':
            moves.extend(gen_knight(p, i, white))
        elif up == 'B':
            moves.extend(gen_bishop(p, i, white))
        elif up == 'R':
            moves.extend(gen_rook(p, i, white))
        elif up == 'Q':
            moves.extend(gen_queen(p, i, white))
        elif up == 'K':
            moves.extend(gen_king(p, i, white))
    return moves
