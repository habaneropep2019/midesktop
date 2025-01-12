#/bin/sh

# Run from root directory of mide, will generate moc files
# for each header file

file_list=`find . -type f -name '*.h'`;
script_dir="$(pwd)"

for input_file_path in $file_list
do
    header_dir_path=$(dirname "${input_file_path}");
    header_file_name="$(basename "${input_file_path}")"
    output_file_name="${header_file_name/.h/.moc}";
    cd $header_dir_path;
    moc $header_file_name > $output_file_name;
    cd $script_dir
done

