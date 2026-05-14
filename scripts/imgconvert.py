#!python3

from PIL import Image, ImageOps
from argparse import ArgumentParser
import sys
import math

SCREEN_WIDTH = 960
SCREEN_HEIGHT = 540

if SCREEN_WIDTH % 2:
    print("image width must be even!", file=sys.stderr)
    sys.exit(1)

parser = ArgumentParser()
parser.add_argument('-i', action="store", dest="inputfile")
parser.add_argument('-n', action="store", dest="name")
parser.add_argument('-o', action="store", dest="outputfile")
parser.add_argument('--gamma', type=float, default=0.92, help='Grayscale gamma correction (default: 0.92). < 1.0 brightens shadows.')
parser.add_argument('--shadow-lift', type=int, default=8, dest='shadow_lift', help='Additional shadow lift in 8-bit range 0..64 (default: 8).')

args = parser.parse_args()

if args.gamma <= 0.0:
    print("gamma must be > 0", file=sys.stderr)
    sys.exit(1)
if args.shadow_lift < 0 or args.shadow_lift > 64:
    print("shadow-lift must be in range 0..64", file=sys.stderr)
    sys.exit(1)


def tone_map_gray(t_value: int, t_gamma: float, t_shadow_lift: int) -> int:
    t_norm = t_value / 255.0
    # Lift shadows progressively: strong in dark regions, minimal in highlights.
    t_lift = (1.0 - t_norm) * (t_shadow_lift / 255.0)
    t_mapped = max(0.0, min(1.0, t_norm + t_lift))
    t_mapped = math.pow(t_mapped, t_gamma)
    return int(round(t_mapped * 255.0))


def quantize_4bit_nearest(t_value: int) -> int:
    # 16 evenly spaced levels in [0..255] mapped to nibble [0..15].
    t_q = int(round(t_value / 17.0))
    return max(0, min(15, t_q))

im = Image.open(args.inputfile)
# convert to grayscale
im = im.convert(mode='L')
im.thumbnail((SCREEN_WIDTH, SCREEN_HEIGHT), Image.LANCZOS)
# Tone mapping before quantization: improve dark detail separation.
im = im.point(lambda t_px: tone_map_gray(t_px, args.gamma, args.shadow_lift))

# Write out the output file.
with open(args.outputfile, 'w') as f:
    f.write("const uint32_t {}_width = {};\n".format(args.name, im.size[0]))
    f.write("const uint32_t {}_height = {};\n".format(args.name, im.size[1]))
    f.write(
        "const uint8_t {}_data[({}*{})/2] = {{\n".format(args.name, math.ceil(im.size[0] / 2) * 2, im.size[1])
    )
    for y in range(0, im.size[1]):
        byte = 0
        done = True
        for x in range(0, im.size[0]):
            l = quantize_4bit_nearest(im.getpixel((x, y)))
            if x % 2 == 0:
                byte = l
                done = False;
            else:
                byte |= (l << 4)
                f.write("0x{:02X}, ".format(byte))
                done = True
        if not done:
            f.write("0x{:02X}, ".format(byte))
        f.write("\n\t");
    f.write("};\n")
