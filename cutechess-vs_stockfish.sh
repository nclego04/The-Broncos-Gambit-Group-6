#!/bin/bash
set -e

cutechess-cli \
  -engine cmd=./engine name="Broncos Gambit" \
  -engine cmd=stockfish name="Stockfish1" option."Skill Level"=1 \
  -each proto=uci tc=10+0.08 \
  -openings file=tests/bratko_kopec.epd format=epd order=random \
  -rounds 5000 \
  -games 2 \
  -concurrency 4 \
  -pgnout tests/tournament.pgn
