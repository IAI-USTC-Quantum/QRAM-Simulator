import subprocess
import json
import os
from pathlib import Path

def run_command(row, timeout=600.0):
    try:
        # Run the command, capturing stdout and stderr
        result = subprocess.run(
            row['cmd'],                  # 命令行
            shell=True,                  # 允许 shell 特性，例如重定向和管道
            text=True,                   # 以文本模式处理输出
            stdout=subprocess.PIPE,      # 捕获标准输出
            stderr=subprocess.PIPE,      # 捕获标准错误
            cwd=row.get('cwd', None),    # 指定工作目录
            timeout=timeout              # 设置超时时间
        )

        # 将输出信息和返回码存入字典中
        row['stdout'] = result.stdout
        row['stderr'] = result.stderr
        row['returncode'] = result.returncode
        row['elapsed'] = timeout if result.returncode != 0 else None

    except subprocess.TimeoutExpired:
        # 处理命令超时
        row['stdout'] = ''
        row['stderr'] = 'Process timed out'
        row['returncode'] = -1
        row['elapsed'] = timeout
    except Exception as e:
        # 处理其他可能的异常
        row['stdout'] = ''
        row['stderr'] = f'Error: {e}'
        row['returncode'] = -1
        row['elapsed'] = timeout

    return row

def save_row_to_file(row, filename):
    try:
        # 检查文件是否存在，如果存在则追加数据，否则创建文件
        if os.path.exists(filename):
            print(f"File already exists: {filename}")
            
        else:
            # 如果文件不存在，创建新的文件并写入数据
            with open(filename, 'w', encoding='utf-8') as file:
                json.dump([row], file, ensure_ascii=False, indent=4)
    except Exception as e:
        print(f"Error while saving row to file: {e}")
        
# Example usage
if __name__ == '__main__':
    
    threads = 8
    root = Path("C:\\Users\\RGZN090201\\Documents\\GitHub\\Quantum-Sparse-State-Calculator\\test\\CPUTest\\QASMTest")
    
    executor = Path("C:\\Users\\RGZN090201\\Documents\\GitHub\\Quantum-Sparse-State-Calculator\\build\\x64-Release\\bin") / "QasmTest.exe"
    input = root / "qasm_files" / "bv_n19.qasm"
    filename = root / "bv_test.json"
    # on windows system
    row = {
        'cmd': f'set OMP_NUM_THREADS={threads}\n {executor} --input {input}', # set OMP_NUM_THREADS=threads
    }


    result = run_command(row)
    print(result)
    save_row_to_file(result, filename)
    print("Command:", result['cmd'])
    print("Stdout:", result['stdout'])
    print("Stderr:", result['stderr'])
    print("Return code:", result['returncode'])
