import subprocess
import pathlib

validation_files = ["test1Validation", "test2Validation", "test3Validation"]
command_line_args = [(str(pathlib.Path().absolute()) + "/scheduler", "1", "disk.in0"), 
(str(pathlib.Path().absolute()) + "/scheduler", "2", "disk.in0", "disk.in1"), 
(str(pathlib.Path().absolute()) + "/scheduler", "3", "disk.in0", "disk.in1", "disk.in2", "disk.in3", "disk.in4"),
(str(pathlib.Path().absolute()) + "/scheduler", "2", "disk.in0", "disk.in1", "disk.in8"),
(str(pathlib.Path().absolute()) + "/scheduler", "5", "disk.in5", "disk.in6", "disk.in7")]
expected_line_length = [5, 9, 21, 9, 45]

for i in range(len(command_line_args)):
    popen  = subprocess.Popen(command_line_args[i], stdout=subprocess.PIPE)
    popen.wait()
    output = popen.stdout.read().decode("utf-8")
    print("--------- test", i, "----------------")
    print("OUTPUT: ")
    print(output)
    output = output.split('\n')
    output.pop()
    if len(output) == expected_line_length[i]:
        print(len(output),"==",expected_line_length[i])
    else:
        print(len(output), "!=", expected_line_length[i])


# test1_validation_file = open(validation_files[2]).read()
# while True:
#     popen  = subprocess.Popen(command_line_args[2], stdout=subprocess.PIPE)
#     popen.wait()
#     output = popen.stdout.read().decode("utf-8")
#     output_split = output.split('\n')
#     if output_split[0][10] == '0' and output_split[1][10] == '1' and output_split[2][10] == '2' \
#     and output_split[17][18] == '0' and output_split[18][18] == '4' and output_split[19][18] == '2':
#         print("--------- test", 2, "----------------")
#         print("OUTPUT: ")
#         print(output) 
#         print("Expected: ")
#         print(test1_validation_file)
#         output = output.split('\n')
#         output.pop()
#         test1_validation_file = test1_validation_file.split('\n')
#         if len(output) == len(test1_validation_file):
#             print(len(output),"==",len(test1_validation_file))
#         else:
#             print(len(output), "!=", len(test1_validation_file))
# 0 1 2 3 4  
#   1