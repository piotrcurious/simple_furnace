from PIL import Image, ImageDraw, ImageFont
import re
import os

def render_vt100(log_file, output_file):
    # Mock screen 40x24 characters
    cols, rows = 40, 24
    char_w, char_h = 10, 18
    img = Image.new('RGB', (cols * char_w, rows * char_h), color=(0, 0, 0))
    draw = ImageDraw.Draw(img)

    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 14)
    except:
        font = ImageFont.load_default()

    # screen[row][col] = (char, color)
    screen = [[(' ', (255, 255, 255)) for _ in range(cols)] for _ in range(rows)]

    color_map = {
        31: (255, 50, 50),   # RED
        32: (50, 255, 50),   # GREEN
        33: (255, 255, 50),  # YELLOW
        34: (80, 80, 255),   # BLUE
        37: (255, 255, 255)  # WHITE
    }

    with open(log_file, "r") as f:
        content = f.read()

    screens = content.split("[CLEAR]")
    if len(screens) < 2: return
    last_screen = screens[-1]

    # We need to parse color changes along with cursor moves
    # Strategy: split by any [ tag
    parts = re.split(r"\[(.*?)\]", last_screen)

    cur_x, cur_y = 0, 0
    cur_color = (255, 255, 255)

    for i in range(0, len(parts), 2):
        text = parts[i]
        # Write text at current cursor with current color
        for char in text:
            if cur_x < cols and cur_y < rows:
                screen[cur_y][cur_x] = (char, cur_color)
                cur_x += 1

        if i + 1 < len(parts):
            tag = parts[i+1]
            if tag.startswith("SET CURSOR"):
                m = re.match(r"SET CURSOR (\d+),(\d+)", tag)
                if m:
                    cur_x, cur_y = int(m.group(1)), int(m.group(2))
            elif tag.startswith("COLOR"):
                m = re.match(r"COLOR (\d+)", tag)
                if m:
                    code = int(m.group(1))
                    cur_color = color_map.get(code, (255, 255, 255))

    # Draw the screen
    for r in range(rows):
        for c in range(cols):
            char, color = screen[r][c]
            draw.text((c * char_w, r * char_h), char, font=font, fill=color)

    img.save(output_file)

if __name__ == "__main__":
    os.makedirs("docs/images", exist_ok=True)
    if os.path.exists("furnace_vt100.ino.log"):
        render_vt100("furnace_vt100.ino.log", "docs/images/vt100_grab.png")
