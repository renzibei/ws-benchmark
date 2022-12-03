import os
import sys
import subprocess
import time
import multiprocessing

def get_work_dir_paths():
    tools_dir_path = os.path.dirname(os.path.realpath(__file__))
    root_dir_path = os.path.dirname(tools_dir_path)
    build_dir_path = os.path.join(root_dir_path, "build")
    return root_dir_path, build_dir_path


def get_lib_names(lib):
    fws_list = ("flashws", "fws")
    uws_list = ("uwebsockets", "uws")
    tpp_list = ("websocketpp", "tpp")
    if lib in fws_list:
        return fws_list
    if lib in uws_list:
        return uws_list
    if lib in tpp_list:
        return tpp_list
    print("lib %s not valid" % lib)
    exit(-1)


def get_server_or_client_exe_path(is_server, lib_name, build_dir_path):
    l_name, s_name = get_lib_names(lib_name)
    build_src_dir_path = os.path.join(build_dir_path, "src")
    build_dir = os.path.join(build_src_dir_path, l_name)
    file_name_self = "echo_server" if is_server else "echo_client"
    s_exe_path = os.path.join(build_dir, s_name + "_" + file_name_self)
    return s_exe_path


def run_one_proc(cmd):
    subprocess.run(cmd)

def test_echo():
    least_arg_num = 9
    if len(sys.argv) < least_arg_num:
        print("Usage:\npython3 tools/bench_ws.py c|s (client or server) "
              "lib_name host_ip host_port"
              " process_cnt "
              " client_cnt_per_proc client_restart_cnt "
              # " min_msg_size=64 max_msg_size=2^21"
              " output_data_file [other arguments for dpdk]")
        return
    c_or_s = sys.argv[1]
    is_server = False
    if c_or_s == "c":
        is_server = False
    elif c_or_s == 's':
        is_server = True
    else:
        print("client or server should be c or s, but input: %s" % c_or_s)
        exit(-1)
    lib_name = sys.argv[2]
    host_ip = sys.argv[3]
    host_port = sys.argv[4]
    process_cnt = int(sys.argv[5])
    print("Process cnt: %d" % process_cnt)

    c_num_per_proc = int(sys.argv[6])
    c_restart_num = int(sys.argv[7])

    # msg_cnt = sys.argv[5]
    output_filename = sys.argv[8]
    # min_msg_size = 64
    # max_msg_size = 1 << 21
    # msg_cnt = 100000
    total_msg_size = 1 << 34
    # if len(sys.argv) >= 7:
    #     min_msg_size = int(sys.argv[6])
    # if len(sys.argv) >= 8:
    #     max_msg_size = int(sys.argv[7])

    if not is_server:

        for i in range(process_cnt):
            temp_filename = output_filename + "_%d" % i
            with open(temp_filename, "w") as out_f:
                out_f.write("msg_size,avg_goodput,P0_latency,P50_latency,P99_latency,"
                            "P999_latency,P100_latency\n")

    root_dir_path, build_dir_path = get_work_dir_paths()
    exe_file_path = get_server_or_client_exe_path(is_server, lib_name=lib_name,
                                                  build_dir_path=build_dir_path)
    print("will run %s" % exe_file_path)

    min_msg_size_log = 6
    # min_msg_size_log = 12
    # max_msg_size_log = 21
    max_msg_size_log = 13
    # if is_server:
    #     msg_size = 1 << max_msg_size_log
    #     arg_list = [exe_file_path, host_ip, host_port, str(msg_size)]
    #     if len(sys.argv) > 7:
    #         arg_list.extend(sys.argv[7:])
    #     subprocess.run(arg_list)
    #     return
    msg_size_list = []
    if is_server:
        msg_size_list.append(1 << max_msg_size_log)
    else:
        msg_size_list = [(1 << i) for i in range(min_msg_size_log, max_msg_size_log + 1)]

    print("msg size will be tested: ", msg_size_list)

    for msg_size in msg_size_list:
        test_msg_cnt = total_msg_size // max(1, (1 << 15) // msg_size) // msg_size

        arg_list_list = []
        for i in range(process_cnt):
            arg_list = []
            if is_server:
                arg_list = [exe_file_path, host_ip, host_port, str(msg_size),
                            output_filename + ("_%d" % i)]
            else:
                arg_list = [exe_file_path, host_ip, host_port, str(msg_size),
                            str(test_msg_cnt),  # msg num per client
                            str(c_num_per_proc),
                            str(c_restart_num),
                            output_filename + ("_%d" % i)]
            if len(sys.argv) > least_arg_num:
                arg_list.extend(sys.argv[least_arg_num:])
            if i == 0:
                arg_list.extend(["--proc-type=primary"])
            else:
                arg_list.extend(["--proc-type=secondary"])
            arg_list.append("--proc-id=%d" % i)
            arg_list_list.append(arg_list)
        # with multiprocessing.Pool(process_cnt) as p:
        #     p.map(run_one_proc, arg_list_list)
        process_list = []
        # first_proc = subprocess.Popen(arg_list_list[0])
        # time.sleep(0.1)
        for i, arg_list in enumerate(arg_list_list):
            print("Will run: ", arg_list)
            proc = subprocess.Popen(arg_list)
            process_list.append(proc)
            if i == 0:
                time.sleep(1.0)
        # reversed to make sure primary proc exit at last
        for i, proc in enumerate(reversed(process_list)):
            proc.wait()
            # if i == 0:
            #     time.sleep(8.0)
        # subprocess.run(arg_list)


def main():
    test_echo()


if __name__ == "__main__":
    main()