from PIL import Image, ImageDraw, ImageFont
import re
import os

def render_vt100(log_file, output_file):
    # Mock screen 40x24 characters
    cols, rows = 40, 24
    char_w, char_h = 10, 18
    img = Image.new('RGB', (cols * char_w, rows * char_h), color=(0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Try to load a mono font
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 14)
    except:
        font = ImageFont.load_default()

    screen = [[' ' for _ in range(cols)] for _ in range(rows)]

    with open(log_file, "r") as f:
        content = f.read()

    # Look for the last screen refresh (starts with [CLEAR])
    screens = content.split("[CLEAR]")
    if len(screens) < 2: return
    last_screen = screens[-1]

    # Simple regex to parse our mocked VT100 codes
    # [SET CURSOR 5,5]Text
    moves = re.split(r"\[SET CURSOR (\d+),(\d+)\]", last_screen)
    # moves[0] is junk, then moves[1]=x, moves[2]=y, moves[3]=text...
    for i in range(1, len(moves), 3):
        x = int(moves[i])
        y = int(moves[i+1])
        text = moves[i+2].split("[")[0] # Stop at next code

        # Write to virtual screen
        for j, char in enumerate(text):
            if x + j < cols and y < rows:
                screen[y][x+j] = char

    # Draw the screen
    for r in range(rows):
        for c in range(cols):
            draw.text((c * char_w, r * char_h), screen[r][c], font=font, fill=(0, 255, 0))

    img.save(output_file)

if __name__ == "__main__":
    os.makedirs("docs/images", exist_ok=True)
    if os.path.exists("furnace_vt100.ino.log"):
        render_vt100("furnace_vt100.ino.log", "docs/images/vt100_grab.png")
