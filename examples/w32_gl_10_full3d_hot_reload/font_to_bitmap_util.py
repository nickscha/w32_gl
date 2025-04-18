from PIL import Image, ImageDraw, ImageFont

def generate_ascii_bitmap(font_path, pixel_height, bmp_output="font_atlas.bmp", c_output="font_data.c"):
    ascii_chars = [chr(i) for i in range(32, 127)]
    num_chars = len(ascii_chars)

    # Load font with possible different style (adjust pixel_height to suit)
    font = ImageFont.truetype(font_path, pixel_height)

    # Determine max bounding box
    max_width = 0
    max_height = 0
    min_offset_x = 0
    min_offset_y = 0

    for ch in ascii_chars:
        bbox = font.getbbox(ch, anchor="lt")  # returns (x0, y0, x1, y1)
        x0, y0, x1, y1 = bbox
        w = x1 - x0
        h = y1 - y0
        max_width = max(max_width, w)
        max_height = max(max_height, h)
        min_offset_x = min(min_offset_x, x0)
        min_offset_y = min(min_offset_y, y0)

    char_width = max_width
    char_height = max_height

    atlas_width = char_width * num_chars
    atlas_height = char_height

    # Create image
    image = Image.new("RGBA", (atlas_width, atlas_height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)

    # Draw each character at fixed spacing
    for i, ch in enumerate(ascii_chars):
        x = i * char_width
        draw.text((x - min_offset_x, -min_offset_y), ch, font=font, fill=(255, 255, 255, 255))

    # Convert the image to grayscale and then to binary (black and white)
    image = image.convert("L")  # Convert to grayscale
    threshold = 128  # Threshold to decide what is black or white (can be adjusted)
    image = image.point(lambda p: p > threshold and 255)  # Apply threshold to make it binary (white and black)

    # Convert the binary image back to "1" mode (1-bit pixels)
    image = image.convert("1")  # Convert to binary image with 1-bit pixels

    # Get the pixel data manually (by accessing pixel values)
    pixel_data = []
    for y in range(atlas_height):
        row = []
        for x in range(atlas_width):
            # Check if the pixel is white (255) or black (0)
            pixel = image.getpixel((x, y))
            row.append(1 if pixel == 255 else 0)  # Store 1 for white and 0 for black
        pixel_data.append(row)

    # Convert the pixel data into a C89 array format
    c_array = []
    for row in pixel_data:
        # Convert each row into a list of bytes where each byte represents 8 pixels
        row_bytes = []
        for i in range(0, len(row), 8):
            byte = 0
            for j in range(8):
                if i + j < len(row) and row[i + j] == 1:
                    byte |= (1 << (7 - j))  # Set the bit
            row_bytes.append(byte)
        c_array.append(row_bytes)

    # Write the C89-compliant static array to a C file without stdlib
    with open(c_output, "w") as f:
        f.write("// Font atlas in C89 style\n")
        f.write("// Generated without using stdlib\n\n")

        f.write("unsigned char font_atlas[] = {\n")

        # Format the C array for output
        for i, row_bytes in enumerate(c_array):
            f.write("    ")
            f.write(", ".join(f"0x{byte:02X}" for byte in row_bytes))
            if i < len(c_array) - 1:
                f.write(",")
            f.write("\n")

        f.write("};\n")
        f.write("\n")
        f.write(f"// Font atlas size: {atlas_width}x{atlas_height}\n")
        f.write(f"// Each glyph size: {char_width}x{char_height}\n")

    # Save BMP image
    image.save(bmp_output)
    print("Saved BMP:", bmp_output)
    print("C89 static array saved to:", c_output)
    print("Each glyph: {}x{}".format(char_width, char_height))
    print("Total atlas: {}x{}".format(atlas_width, atlas_height))

if __name__ == "__main__":
    font_path = "C:\\Windows\\Fonts\\consola.ttf"  # Consider testing a different font file here.
    generate_ascii_bitmap(font_path, pixel_height=32)
