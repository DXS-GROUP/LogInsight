import os

def get_file_info(file_path):
    """Gets information about the file: size, date of creation and date of last modification."""
    file_info = os.stat(file_path)
    return {
        "size": file_info.st_size,
        "creation_time": file_info.st_ctime,
        "modification_time": file_info.st_mtime,
    }

def read_lines(file_path):
    """Reads lines from a file."""
    with open(file_path, 'r') as f:
        return f.readlines()
