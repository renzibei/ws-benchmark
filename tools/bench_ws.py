import os
import sys
import subprocess


def get_work_dir_paths():
    tools_dir_path = os.path.dirname(os.path.realpath(__file__))
    root_dir_path = os.path.dirname(tools_dir_path)
    build_dir_path = os.path.join(root_dir_path, "build")
    return root_dir_path, build_dir_path


def get_lib_names(lib):
    fws_list = ("flashws", "fws")
    uws_list = ("uwebsockets", "uws")
    if lib in fws_list:
        return fws_list
    if lib in uws_list:
        return uws_list
    print("lib %s not valid" % lib)
    exit(-1)


def get_server_or_client_exe_path(is_server, lib_name, build_dir_path):
    l_name, s_name = get_lib_names(lib_name)
    build_src_dir_path = os.path.join(build_dir_path, "src")
    build_dir = os.path.join(build_src_dir_path, l_name)
    file_name_self = "echo_server" if is_server else "echo_client"
    s_exe_path = os.path.join(build_dir, s_name + "_" + file_name_self)
    return s_exe_path


def test_echo():
    if len(sys.argv) < 6:
        print("Usage:\npython3 tools/bench_ws.py c|s (client or server) "
              "lib_name host_ip host_port"
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
    # msg_cnt = sys.argv[5]
    output_filename = sys.argv[5]
    # min_msg_size = 64
    # max_msg_size = 1 << 21
    # msg_cnt = 100000
    total_msg_size = 1 << 34
    # if len(sys.argv) >= 7:
    #     min_msg_size = int(sys.argv[6])
    # if len(sys.argv) >= 8:
    #     max_msg_size = int(sys.argv[7])

    if not is_server:
        with open(output_filename, "w") as out_f:
            out_f.write("msg_size,avg_goodput,P0_latency,P50_latency,P99_latency,"
                        "P999_latency,P100_latency\n")

    root_dir_path, build_dir_path = get_work_dir_paths()
    exe_file_path = get_server_or_client_exe_path(is_server, lib_name=lib_name,
                                                  build_dir_path=build_dir_path)
    print("will run %s" % exe_file_path)

    min_msg_size_log = 6
    max_msg_size_log = 21
    if is_server:
        msg_size = 1 << max_msg_size_log
        arg_list = [exe_file_path, host_ip, host_port, str(msg_size)]
        if len(sys.argv) > 6:
            arg_list.extend(sys.argv[6:])
        subprocess.run(arg_list)
        return

    msg_size_list = [(1 << i) for i in range(min_msg_size_log, max_msg_size_log + 1)]

    print("msg size will be tested: ", msg_size_list)

    for msg_size in msg_size_list:
        test_msg_cnt = total_msg_size / max(1, (1 << 15) // msg_size) / msg_size
        arg_list = [exe_file_path, host_ip, host_port, str(msg_size),
                    str(test_msg_cnt), output_filename]
        if len(sys.argv) > 6:
            arg_list.extend(sys.argv[6:])
        subprocess.run(arg_list)


def main():
    test_echo()


if __name__ == "__main__":
    main()