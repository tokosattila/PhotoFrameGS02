fontconvert
============

Prerequisite you need to install python3 and install `freetype-py` using pip
The approximate process is like this:

.. code-block:: shell

    sudo apt install python3-pip
    python3 -m pip install freetype-py

The previous is the prerequisite for implementation, and then you need to store the font file you want to convert in the same path as fontconvert.py. This is just for more convenient operation. You can also fill in the font path.

Then just follow the command below to convert the font.

.. code-block:: shell

    python3 fontconvert.py --compress demo 16 msyh.ttc > font.h

Explanation of specific parameters:

.. code-block:: shell

    python3 fontconvert.py --compress [generated font name] [font size] [font file path] > [generated font file]

Of course, this only demonstrates the generation of standard ascii codes. If you need other fonts, you only need to fill in the unicode encoding of the font to be generated in the `fontconvert.py`  `intervals` list.

Please make sure that the unicode encoding in the fontconvert.py intervals list is included in your font file, otherwise please comment other encodings and only keep the 32,126 range!

.. image:: https://user-images.githubusercontent.com/22990954/104092951-446c1880-52c2-11eb-852e-8c50626eb340.png

imgconvert
============

1. Prerequisites

.. code-block:: shell

    python3 -m pip install pillow

2. Instructions

.. code-block:: shell

    imgconvert.py [-h] [-i INPUTFILE] [-n NAME] [-o OUTPUTFILE]

Optional tone tuning parameters:

- `--gamma` (default: `0.92`) - values below `1.0` brighten darker tones.
- `--shadow-lift` (default: `8`, range `0..64`) - lifts near-black shades to preserve detail in 16-level output.

Recommended presets for 16-level grayscale e-paper:

- Balanced (default): `--gamma 0.92 --shadow-lift 8`
- Dark-detail boost: `--gamma 0.88 --shadow-lift 12`
- Mild boost: `--gamma 0.95 --shadow-lift 6`

3. examples:

.. code-block:: shell

    python imgconvert.py -i demo.png  -n demo -o demo.h

    # If dark shades are still too compressed on the panel:
    python imgconvert.py -i demo.png -n demo -o demo.h --gamma 0.88 --shadow-lift 12

    # Mild correction profile:
    python imgconvert.py -i demo.png -n demo -o demo.h --gamma 0.95 --shadow-lift 6

imgprepare
============

Converts input images (JPG/JPEG/PNG/WEBP) into grayscale JPG files for
the 4.7" grayscale e-paper panel. The converter center-crops to the target
aspect ratio and then resizes to final resolution.

Note: default output keeps 8-bit grayscale detail. Use `--quantize-16` only if
you explicitly want pre-quantized 16-level output.

1. Prerequisites

.. code-block:: shell

    python3 -m pip install pillow

2. Instructions

.. code-block:: shell

    imgprepare/imgprepare.py [-h] (-i INPUT | --input-dir INPUT_DIR)
                  [-o OUTPUT | --output-dir OUTPUT_DIR]
                  [--recursive] [--size WIDTHxHEIGHT] [--portrait]
                  [--quality 1..100] [--quantize-16]

Default folders:

- `scripts/imgprepare/input` for source images
- `scripts/imgprepare/output` for converted files

3. Examples

.. code-block:: shell

    # Single file, default landscape 960x540
    python imgprepare/imgprepare.py -i imgprepare/input/demo.webp -o imgprepare/output/demo.jpg

    # Batch conversion with per-image progress
    python imgprepare/imgprepare.py --input-dir imgprepare/input --output-dir imgprepare/output --recursive

    # Optional custom size (landscape by default)
    python imgprepare/imgprepare.py --input-dir imgprepare/input --output-dir imgprepare/output --size 960x540

    # Optional legacy mode: force 16-level pre-quantized JPG output
    python imgprepare/imgprepare.py --input-dir imgprepare/input --output-dir imgprepare/output --recursive --quantize-16
