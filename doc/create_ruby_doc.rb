#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :

require 'rdoc/rdoc'

module RDoc
    #define these ourself, as they are in all files.
#    KNOWN_CLASSES['rb_mPaludis'] = 'Paludis'
#    KNOWN_CLASSES['rb_mQA'] = 'Paludis::QA'
#    KNOWN_CLASSES['c_environment'] = 'Paludis::Environment'

    class C_Parser_Paludis < C_Parser
        #override C_Parse
        parse_files_matching(/\.(c|cc|cpp|CC)$/)

        def initialize(top_level, file_name, body, options, stats)
            #Paludis and Paludis::QA are already added
            body.gsub!('paludis_module()','c_paludis_module')
            body.gsub!('paludis_qa_module()','c_paludis_qa_module')
            body.gsub!('no_config_environment_class()','c_no_config_environment')
            body.gsub!('environment_class()','c_environment')

            #parse_c hates rb_defines over multiple lines
            body.gsub!(/(rb_define[^;]+)\n/)  {|match| $1}
            new_body=''
            new_body += "\n" + 'c_paludis_module = rb_define_module("Paludis");' + "\n"
            new_body += 'c_paludis_qa_module = rb_define_module_under(c_paludis_module, "QA");' + "\n"
            new_body += 'c_environment = rb_define_class_under(c_paludis_module, "Environment", rb_cObject);' + "\n"
            new_body += 'c_no_config_environment = rb_define_class_under(c_paludis_module, "NoConfigEnvironment", c_environment);' + "\n"
            body.each_line do |line|
                next if line =~ /rb_mPaludis\s*=/
                next if line =~ /rb_mQA\s*=/
                next if line =~ /c_environment\s*=/
                next if line =~ /c_no_config_environment\s*=/
                if line =~ /cc_enum_special/
                    line.scan(/cc_enum_special<([^,]+),\s*([^,]+),\s*([^>]+)>/) do
                        |header_file, type, in_class|
                        new_body+= generate_consts(header_file, type, in_class)
                    end
                    next
                end
                if line =~ /rb_define/
                    #help rdoc recognise normal methods
                    line.gsub!('&','')
                    line.gsub!(/RUBY_FUNC_CAST\(*([^)]+)\)*/) {|match| $1}
                    #help rdoc recognise template methods
                    line.gsub!(/\w+<\s*\w+(::\w+)*(,\s*\w+)*(::\w+)?>::\w+/,'template_methods')
                end
                new_body+= line
            end
            #puts new_body
            #exit
            super(top_level, file_name, new_body, options, stats)
        end

        def generate_consts(header, type, in_class)
            consts = []
            file = File.read("../#{header}")
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
  r = RDoc::RDoc.new
  r.document(ARGV)
rescue RDoc::RDocError => e
  $stderr.puts e.message
  exit(1)
end
