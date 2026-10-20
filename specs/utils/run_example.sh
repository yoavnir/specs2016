#!/bin/bash
#
# Render example.spec with the specs syntax definition, so that changes to
# specs.xml can be eyeballed. Run it from this directory.
#
# Two things are easy to get wrong here:
#
#  - The specification has to be wrapped in a fenced code block tagged
#    "specs". Handing a bare .spec file to pandoc gets it read as Markdown,
#    where the comment lines become headings and the indented spec units
#    become plain indented code blocks, so nothing is highlighted at all.
#  - The style has to be specs.theme, the one the guidebook is built with.
#    The built-in styles leave many of the default styles used by specs.xml
#    undefined, and those tokens then come out plain black. See the
#    "Guidebook Highlighting Theme" section of the top-level AGENTS.md.

set -e

xmllint --noout specs.xml

fenced() { echo '```specs'; cat example.spec; echo '```'; }

# HTML. The -s (standalone) switch is what makes the output colourful:
# without it pandoc emits the <span class="..."> markup but none of the CSS.
fenced | pandoc -s --syntax-definition=specs.xml --highlight-style specs.theme \
                --metadata pagetitle=example -o example.html

# PDF, with the guidebook's document class, fonts and header, so that the
# result matches how the specification will look in the guidebook.
fenced | pandoc --pdf-engine=xelatex --syntax-definition=specs.xml \
                --highlight-style specs.theme \
                -V documentclass=book -V classoption=openany \
                -V mainfont="DejaVu Serif" -V monofont="DejaVu Sans Mono" \
                -H ../docs/resources/header.tex -o example.pdf

echo "wrote example.html and example.pdf"
