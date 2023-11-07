How to use the production tool:

1)The Production tool input is a file indicated by the 
  argument -s, followed by the path to the file.
  For example if my file is called "input" and it resides
  in the "/home" directory the flag would look like this : -s/home/input

2)The production tool output is a file indicated by the
  argument -d, followed by the path to the file.
  For example if my file is called "output" and it resides
  in the "/home" directory the flag would look like this : -d/home/output

3)The Production tool is capable of two actions:
    
    i)Creating a binary configuration file from a text configuration file. 
      The argument for this action is -acreate_config
    
    ii)Creating a binary text file from a binary configuration file. 
      The argument for this action is -aparse_config

4)For example, if we had a text configuration file "our_configuration.txt" which
  was in the "/user/dani/" directory and we wanted to create a binary configuration file 
  called "our_binary_configuration.bin" that would reside in the "/user/dani/Desktop/" 
  directory we would use the software the following way:

  On Windows:

    ./production_tool.exe -s/user/dani/our_configuration.txt -d/user/dani/Desktop/our_binary_configuration.bin -acreate_config

  On Linux:

   ./production_tool -s/user/dani/our_configuration.txt -d/user/dani/Desktop/our_binary_configuration.bin -acreate_config

  If we had a binary configuration file "binary_configuration_file.bin" which
  was in the "/user/dani/Downloads/" directory and we wanted to create a text configuration file 
  called "our_text_configuration.txt" that would reside in our current working 
  directory we would use the software the following way:

  On Windows:

    ./production_tool.exe -s/user/dani/Downloads/binary_configuration_file.bin -dour_text_configuration.txt -aparse_config

  On Linux:

   ./production_tool -s/user/dani/Downloads/binary_configuration_file.bin -dour_text_configuration.txt -aparse_config

5)If we want to check the current version of the tool:

    On Windows:

    ./production_tool.exe -v
    or
    ./production_tool.exe --version

    On Linux:

    ./production_tool -v
    or
    ./production_tool --version

6)To get a reminder about the agruments usage of the software:

    On Windows:

    ./production_tool.exe -h
    or
    ./production_tool.exe --help

    On Linux:

    ./production_tool -h
    or
    ./production_tool --help


    


How to compile the production tool from source: 

On Windows:
1 Download and install cygwin
2 Download and install mingw64
3 Add mingw path to the Path variables (in Control panel)
4 Run cygwin
5 write "cd \>production_folder_path\"
6 make

On Linux:
1 Navigate to the production folder
2 make
