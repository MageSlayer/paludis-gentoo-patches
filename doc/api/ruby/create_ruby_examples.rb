#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'rubygems'
require 'syntax/convertors/html'

convertor = Syntax::Convertors::HTML.for_syntax('ruby')

srcdir = ARGV.shift

topuri = '../../'
toplinks = File.read(srcdir + '/../toplinks.html.part.in')
header = File.read(srcdir + '/../../header.html.part.in')
footer = File.read(srcdir + '/../../footer.html.part')
css = '<link rel="stylesheet" href="ruby_syntax.css" type="text/css" />'
header.gsub!('###TOPLINKS###', toplinks)
header.gsub!('###TOPURI###', topuri)
header.gsub!("</head>", "#{css}</head>")

ARGV.each do |example_file|
    html = convertor.convert(File.read(srcdir + '/' + example_file), false)
    File.open('ruby/' + example_file.gsub(/rb$/,'html'), 'w') do |output|
        output.write header
        output.write "<h1>#{example_file}</h1>"

        #Grab examples description
        html.scan(/=begin description(.*?)=end/m) {|desc| output.puts "<p>#{desc[0]}</p>"}

        #enclose each line of an =begin block in a comment span
        html.gsub!(/=begin description.*?=end/m) do |match|
            match.gsub("\n", "</span>\n<span class=\"comment\">")
        end
        output.write '<pre>';
        i = 0
        html.each_line do |line|
            i += 1
            output.print '<span class="lineno">' + i.to_s.rjust(5,'0') + '</span> <span class="ruby_code">' + line + '</span>'
        end
        output.write '</pre>'
        output.write footer
    end
end

