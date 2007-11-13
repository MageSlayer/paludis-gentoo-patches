#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

import sys, os, re
from pygments import highlight
from pygments.lexers import PythonLexer
from pygments.formatters import HtmlFormatter

srcdir = sys.argv[1]

lexer = PythonLexer()
formatter = HtmlFormatter(linenos=True, cssclass="syntax")

topuri = '../../'
toplinks = file(os.path.join(srcdir, '../toplinks.html.part.in')).read()
header = file(os.path.join(srcdir, '../../header.html.part.in')).read()
footer = file(os.path.join(srcdir, '../../footer.html.part.in')).read()
css = '<link rel="stylesheet" href="python_syntax.css" type="text/css" />'

header = header.replace('###TOPLINKS###', toplinks)
header = header.replace('###TOPURI###', topuri)
header = header.replace('</head>', css + '</head>')

for example in sys.argv[2:]:
    input = file(os.path.join(srcdir, example)).read()

    example_html = "python/" + example.replace(".py", ".html")

    out_file = file(example_html, "w")
    out_file.write(header)
    out_file.write("<h1>%s</h1>" % example)
    out_file.write("<p>%s</h1>" % re.search('"""(.*?)"""', input).groups()[0])
    out_file.close()

    highlight(input, lexer, formatter, file(example_html, "a"))

    file(example_html, "a").write(footer)
