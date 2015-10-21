#!/usr/bin/env python
import click
import re


def match(m, code=False):
    return {"code": code,
            "start": m.start(),
            "content": m.group()}


def codify(txt):
    print(".. code-block:: cpp\n\n")
    for line in txt.split("\n"):
        print("  " + line)
    print("\n")


@click.command()
@click.argument('input', type=click.File('r'))
def extract(input):
    """
    Generate RST file from code file
    Restructured Text is contained inside
    /**rst
    */
    block

    Code examples are contained inside
    ///```cpp
    ///```
    block
    """
    MDK_REGEX = r"(?<=^\/\*{2}rst)(.*?)(?=^\*\/)"
    CODE_REGEX = r"(?<=^\/{3}```cpp\n)(.*?)(?=^\/{3}```)"
    mdk_seq = re.compile(MDK_REGEX, flags=re.MULTILINE + re.DOTALL)
    data = input.read()
    blocks = []
    for m in mdk_seq.finditer(data):
        blocks.append(match(m))
    mdk_seq = re.compile(CODE_REGEX, flags=re.MULTILINE + re.DOTALL)
    for m in mdk_seq.finditer(data):
        blocks.append(match(m, True))
    blocks.sort(key=lambda x: x["start"])
    for block in blocks:
        print(".. This document is generated with docs/extract.py and should not be modified by hand")
        if block["code"]:
            codify(block["content"])
        else:
            print(block["content"])

if __name__ == '__main__':
    extract()
