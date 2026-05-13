#!python3

"""
Prepare photos for LilyGo T5 4.7" grayscale e-paper panel.

Features:
- Input formats: JPG/JPEG, PNG, WEBP
- Output: grayscale JPG (8-bit by default; optional 16-level pre-quantization)
- Center-crop to target aspect ratio, then resize
- Single-file and batch directory processing
- Per-image progress output

Default target is landscape 960x540 (panel native resolution).
"""

from __future__ import annotations

from argparse import ArgumentParser
from pathlib import Path
from typing import List, Tuple

from PIL import Image


SUPPORTED_EXTENSIONS = {".jpg", ".jpeg", ".png", ".webp"}
DEFAULT_WIDTH = 960
DEFAULT_HEIGHT = 540


def parse_size(value: str) -> Tuple[int, int]:
  text = value.lower().strip()
  if "x" not in text:
    raise ValueError("size must be in WIDTHxHEIGHT format, e.g. 960x540")
  width_text, height_text = text.split("x", 1)
  width = int(width_text)
  height = int(height_text)
  if width <= 0 or height <= 0:
    raise ValueError("size values must be positive")
  return width, height


def is_supported_file(path: Path) -> bool:
  return path.is_file() and path.suffix.lower() in SUPPORTED_EXTENSIONS


def collect_inputs(input_dir: Path, recursive: bool) -> List[Path]:
  pattern = "**/*" if recursive else "*"
  files = [path for path in input_dir.glob(pattern) if is_supported_file(path)]
  files.sort()
  return files


def center_crop_to_aspect(image: Image.Image, target_w: int, target_h: int) -> Image.Image:
  src_w, src_h = image.size
  src_ratio = src_w / src_h
  target_ratio = target_w / target_h

  if src_ratio > target_ratio:
    # Too wide: crop left/right.
    new_w = int(round(src_h * target_ratio))
    left = (src_w - new_w) // 2
    box = (left, 0, left + new_w, src_h)
  else:
    # Too tall: crop top/bottom.
    new_h = int(round(src_w / target_ratio))
    top = (src_h - new_h) // 2
    box = (0, top, src_w, top + new_h)

  return image.crop(box)


def quantize_to_16_levels(gray_image: Image.Image) -> Image.Image:
  # Map each pixel to one of 16 evenly spaced levels: 0, 17, ..., 255.
  return gray_image.point(lambda value: max(0, min(255, int(round(value / 17.0)) * 17)))


def process_image(
  input_path: Path,
  output_path: Path,
  target_w: int,
  target_h: int,
  quality: int,
  quantize16: bool,
) -> None:
  with Image.open(input_path) as image:
    image = image.convert("RGB")
    cropped = center_crop_to_aspect(image, target_w, target_h)
    resized = cropped.resize((target_w, target_h), Image.Resampling.LANCZOS)
    gray = resized.convert("L")
    output_image = quantize_to_16_levels(gray) if quantize16 else gray

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_image.save(output_path, format="JPEG", quality=quality, optimize=True)


def output_name_for_input(input_path: Path) -> str:
  return f"{input_path.stem}.jpg"


def build_parser() -> ArgumentParser:
  parser = ArgumentParser(description="Convert images to e-paper-ready grayscale JPG.")

  source_group = parser.add_mutually_exclusive_group(required=True)
  source_group.add_argument("-i", "--input", dest="input_file", help="Single input image path.")
  source_group.add_argument("--input-dir", dest="input_dir", help="Input directory for batch conversion.")

  parser.add_argument("-o", "--output", dest="output_file", help="Output JPG path for single-file mode.")
  parser.add_argument("--output-dir", dest="output_dir", help="Output directory (required for batch mode).")
  parser.add_argument("--recursive", action="store_true", help="Recursively scan input-dir.")
  parser.add_argument("--size", default=f"{DEFAULT_WIDTH}x{DEFAULT_HEIGHT}", help="Target size WIDTHxHEIGHT (default: 960x540).")
  parser.add_argument("--portrait", action="store_true", help="Shortcut for 540x960 target.")
  parser.add_argument("--quality", type=int, default=92, help="JPEG quality 1..100 (default: 92).")
  parser.add_argument("--quantize-16", action="store_true", help="Optional pre-quantization to 16 grayscale levels before JPEG save.")

  return parser


def main() -> int:
  parser = build_parser()
  args = parser.parse_args()

  try:
    target_w, target_h = parse_size(args.size)
  except Exception as error:
    print(f"ERROR: {error}")
    return 2

  if args.portrait:
    target_w, target_h = target_h, target_w

  if args.quality < 1 or args.quality > 100:
    print("ERROR: --quality must be between 1 and 100")
    return 2

  if args.input_file:
    input_path = Path(args.input_file)
    if not is_supported_file(input_path):
      print("ERROR: input file is missing or unsupported (use jpg/jpeg/png/webp)")
      return 2

    if args.output_file:
      output_path = Path(args.output_file)
    elif args.output_dir:
      output_path = Path(args.output_dir) / output_name_for_input(input_path)
    else:
      print("ERROR: single-file mode requires --output or --output-dir")
      return 2

    print(f"[1/1] {input_path.name} -> {output_path.name}")
    process_image(input_path, output_path, target_w, target_h, args.quality, args.quantize_16)
    print(f"DONE: {output_path}")
    return 0

  input_dir = Path(args.input_dir)
  if not input_dir.exists() or not input_dir.is_dir():
    print("ERROR: --input-dir does not exist or is not a directory")
    return 2

  if not args.output_dir:
    print("ERROR: batch mode requires --output-dir")
    return 2

  output_dir = Path(args.output_dir)
  inputs = collect_inputs(input_dir, args.recursive)
  if not inputs:
    print("No supported image found in input directory.")
    return 0

  total = len(inputs)
  mode_text = "grayscale-16 JPG" if args.quantize_16 else "grayscale JPG"
  print(f"Converting {total} image(s) to {target_w}x{target_h} {mode_text}...")
  for index, input_path in enumerate(inputs, start=1):
    relative_path = input_path.relative_to(input_dir)
    output_path = output_dir / relative_path.with_suffix(".jpg")
    progress = int((index * 100) / total)
    print(f"[{index}/{total}] ({progress:3d}%) {relative_path}")
    process_image(input_path, output_path, target_w, target_h, args.quality, args.quantize_16)

  print(f"DONE: {total} image(s) written to {output_dir}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
