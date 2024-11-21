import os


def convert_to_cpp_array(file_path, array_name):
    with open(file_path, "rb") as file:
        byte_array = file.read()

    # Gera o código C++ para o array
    cpp_array = f"const unsigned char {array_name}[] PROGMEM = {{\n"
    cpp_array += ", ".join(f"0x{byte:02x}" for byte in byte_array)
    cpp_array += "\n};\n"
    cpp_array += f"const unsigned int {array_name}_len = {len(byte_array)};\n"

    return cpp_array


# Verifica se o arquivo de saída já existe e adiciona "_new" se necessário
def get_output_file_path(base_name, extension):
    new_output_file_path = f"{base_name}{extension}"
    if os.path.exists(new_output_file_path):
        new_output_file_path = f"{base_name}_new{extension}"
    return new_output_file_path


# Exemplo de uso
files_to_convert = [
    # ("cmd.html", "cmd_html"),
    # ("favicon.ico", "favicon_ico"),
    # ("index.html", "index_html"),
    # ("info.html", "info_html"),
    # ("ota.html", "ota_html"),
    # ("script.js", "script_js"),
    # ("serial.html", "serial_html"),
    # ("sha.js", "sha_js"),
    # ("style.css", "style_css"),
]

for file_name, array_name in files_to_convert:
    cpp_code = convert_to_cpp_array(file_name, array_name)
    base_name, extension = os.path.splitext(f"{array_name}.h")
    new_output_file_path = get_output_file_path(base_name, extension)
    with open(new_output_file_path, "w") as output_file:
        output_file.write(cpp_code)
