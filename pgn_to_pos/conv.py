import chess.pgn

with open("game.pgn", "r") as input:
    game = chess.pgn.read_game(input)

# Extract only the mainline moves
moves = []
board = game.board()
initial = str(board.fen())
for node in game.mainline():
    moves.append(str(node.move))
    board.push(node.move)

res = ""
for i in range(0, len(moves), 1):
    move = moves[i]
    res += move.strip() + " "

with open("output.txt", "w") as output:
    output.write("position fen " + initial + " moves ")
    output.write(res.strip())