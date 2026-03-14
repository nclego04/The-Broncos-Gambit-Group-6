def legal_moves(p):
    out = []
    for m in pseudo_legal_moves(p):
        np = make_move(p, m)
        if not in_check(np, not np.white_to_move):
            out.append(m)
    return out