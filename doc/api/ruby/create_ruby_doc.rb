#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'rubygems'
require 'rdoc/rdoc'

gps = Gem::GemPathSearcher.new;
allison_spec = gps.find('allison.rb');

allison = "#{allison_spec.loaded_from.gsub('specifications','gems').gsub('.gemspec','')}/lib/allison.rb"

module RDoc

    class C_Parser_Paludis < C_Parser
        #override C_Parse
        parse_files_matching(/\.(c|cc|cpp|CC)$/)

        def initialize(top_level, file_name, body, options, stats)
            paludis_desc = <<-DESC

                /*
                 * Document-module: Paludis
                 *
                 * <b>Paludis</b> is the other package mangler, this is the doc to the ruby binding. The C++ library
                 * documentation may also help.
                 *
                 */
                c_paludis_module = rb_define_module("Paludis");

            DESC
            #Paludis and are already added
            body.gsub!('paludis_module()','c_paludis_module')

            #parse_c hates rb_defines over multiple lines
            body.gsub!(/(rb_define[^;]+)\n/)  {|match| $1}
            new_body=''
            new_body += paludis_desc
            body.each_line do |line|
                next if line =~ /c_paludis_module\s*=/
                if line =~ /cc_enum_special/
                    line.scan(/cc_enum_special<([^,]+),\s*([^,]+),\s*([^>]+)>/) do
                        |header_file, type, in_class|
                        new_body+= generate_consts(header_file, type, in_class)
                    end
                    next
                end
                line.gsub!(/FAKE_RDOC_METHOD\(([a-zA-Z0-9_\-]+)\);/) {|match| "VALUE #{$1}(VALUE self) { }"}
                if line =~ /rb_define/
                    #help rdoc recognise normal methods
                    line.gsub!('&','')
                    line.gsub!(/RUBY_FUNC_CAST\(*([^)]+)\)*/) {|match| $1}
                    line.gsub!(/RDOC_IS_STUPID\((\w+),.*, ([0-9\-])+\);/) {|match| "#{$1}, #{$2})"}
                    #help rdoc recognise template methods
                    line.gsub!(/[a-zA-Z0-9\-_:]+<\s*[a-zA-Z0-9\-_:]+(::[a-zA-Z0-9\-_:]+)*(,\s*[a-zA-Z0-9\-_:]+)*(::[a-zA-Z0-9\-_:]+)?>::[a-zA-Z0-9\-_:]+/,'template_methods')
                    line.gsub!(/[a-zA-Z0-9\-_:]+\s*<\s*[a-zA-Z0-9\-_:]+\s*<\s*[a-zA-Z0-9\-_:]+\s*>,\s*[a-zA-Z0-9\-_:]+::[a-zA-Z0-9\-_:]+>::[a-zA-Z0-9\-_:]+/, 'template_methods')
                    line.gsub!(/[a-zA-Z0-9\-_:]+\s*<\s*[a-zA-Z0-9\-_:]+\s*<\s*[a-zA-Z0-9\-_:]+\s*<[^>]+>\s*>,\s*[a-zA-Z0-9\-_:]+::[a-zA-Z0-9\-_:]+>::[a-zA-Z0-9\-_:]+/, 'template_methods')
                end
                new_body+= line
            end
            super(top_level, file_name, new_body, options, stats)
        end

        def generate_consts(header, type, in_class)
            consts = []
            file = File.read(ENV["TOP_SRCDIR"] + "/#{header}")

            match = Regexp.new(/enum\s+#{type}\s+\{([^}]+)\}/)#, Regexp::MULTILINE)
            if file =~ match
                enum = $1
                last = enum[Regexp.new(/\slast_(\w+)[\s,]/),1]
                i = 0
                enum.each_line do |line|
                    next if line =~/last/
                    next if line.strip == ''
                    next unless line =~ /,/
                    (const, comment) = line.split(',',2)
                    const.strip!
                    comment.strip!
                    comment.gsub!(%r{^[^a-zA-Z0-9]*},'')
                    const.sub!("#{last}_",'').capitalize! #strip start
                    const.gsub!(%r{[_\s](\w)}) { |x| $1.capitalize}
                    consts << "/*\n*#{comment}\n*/\nrb_define_const(#{in_class}, \"#{const}\", #{i});"
                    i+=1
                end
            end
            consts.join("\n")
        end
    end
end
begin
  ARGV << '--template' << allison
  r = RDoc::RDoc.new
  r.document(ARGV)
rescue RDoc::RDocError => e
  $stderr.puts e.message
  exit(1)
end
