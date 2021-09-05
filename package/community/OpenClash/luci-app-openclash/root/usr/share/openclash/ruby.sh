#!/bin/sh

ruby_read()
{
local Value RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); puts Value$2}.join"
if [ -n "$(echo "$2" |grep '.to_yaml' 2>/dev/null)" ]; then
   ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null |sed '1d' 2>/dev/null
else
   ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
fi
}

ruby_read_hash()
{
local Value RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = $1; puts Value$2}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

ruby_edit()
{
local Value RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); Value$2; File.open('$1','w') {|f| YAML.dump(Value, f)}}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

#数组覆盖
ruby_cover()
{
local Value Value_1 RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); if File::exist?('$3') then Value_1 = YAML.load_file('$3'); if not '$4'.empty? then Value$2=Value_1['$4']; else Value$2=Value_1 end else if not '$4'.empty? then Value.delete('$4'); end; end; File.open('$1','w') {|f| YAML.dump(Value, f)}}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

#hash增加
ruby_merge()
{
local Value Value_1 RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); Value_1 = YAML.load_file('$3'); Value$2.merge!(Value_1$4); File.open('$1','w') {|f| YAML.dump(Value, f)}}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

#hash去重
ruby_uniq()
{
local Value RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); Value$2=Value$2.uniq; File.open('$1','w') {|f| YAML.dump(Value, f)}}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

#数组指定位置前添加一组值(不要key)
ruby_arr_add_file()
{
local Value Value_1 RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); Value_1 = YAML.load_file('$4').reverse!; Value_1$5.each{|x| Value$2.insert($3,x)}; Value$2=Value$2.uniq; File.open('$1','w') {|f| YAML.dump(Value, f)}}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

#数组开头添加一组值(含key)
ruby_arr_head_add_file()
{
local Value Value_1 RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); Value_1 = YAML.load_file('$3'); Value$2=(Value_1$4+Value$2).uniq; File.open('$1','w') {|f| YAML.dump(Value, f)}}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

#数组指定位置前增加值
ruby_arr_insert()
{
local Value RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); Value$2=Value$2.insert($3,'$4').uniq; File.open('$1','w') {|f| YAML.dump(Value, f)}}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}

ruby_read_hash_arr()
{
local Value RUBY_YAML_PARSE
if [ -z "$1" ] || [ -z "$2" ]; then
	return
fi
RUBY_YAML_PARSE="Thread.new{Value = YAML.load_file('$1'); Value$2.each do |i| puts i$3 end}.join"
ruby -ryaml -E UTF-8 -e "$RUBY_YAML_PARSE" 2>/dev/null
}