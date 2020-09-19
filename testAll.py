import subprocess
import pathlib

validation_files = ["test1Validation", "test2Validation", "test3Validation"]
command_line_args = [(str(pathlib.Path().absolute()) + "/scheduler", "1", "disk.in0"), 
(str(pathlib.Path().absolute()) + "/scheduler", "2", "disk.in0", "disk.in1"), 
(str(pathlib.Path().absolute()) + "/scheduler", "3", "disk.in0", "disk.in1", "disk.in2", "disk.in3", "disk.in4")]

for i in range(3):
    popen  = subprocess.Popen(command_line_args[i], stdout=subprocess.PIPE)
    popen.wait()
    output = popen.stdout.read().decode("utf-8")
    test1_validation_file = open(validation_files[i]).read()
    print("--------- test", i, "----------------")
    print("OUTPUT: ")
    print(output)
    print("Expected: ")
    print(test1_validation_file)
    output = output.split('\n')
    output.pop()
    test1_validation_file = test1_validation_file.split('\n')
    if len(output) == len(test1_validation_file):
        print(len(output),"==",len(test1_validation_file))
    else:
        print(len(output), "!=", len(test1_validation_file))
