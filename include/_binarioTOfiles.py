import os


def convert_from_cpp_array(array_file_path, output_file_path):
    with open(array_file_path, "r") as array_file:
        lines = array_file.readlines()

    # Encontra a linha que contém os dados do array
    array_data = []
    inside_array = False
    for line in lines:
        if "{" in line:
            inside_array = True
        elif "}" in line:
            inside_array = False
        elif inside_array:
            array_data.extend(line.strip().split(", "))

    # Converte os dados do array de volta para bytes
    byte_array = bytearray(
        int(byte, 16) for byte in array_data if byte.startswith("0x")
    )

    # Verifica se o arquivo de saída já existe e adiciona "_new" se necessário
    base_name, extension = os.path.splitext(output_file_path)
    new_output_file_path = output_file_path
    if os.path.exists(new_output_file_path):
        new_output_file_path = f"{base_name}_new{extension}"

    # Escreve os bytes no arquivo de saída
    with open(new_output_file_path, "wb") as output_file:
        output_file.write(byte_array)


# Exemplo de uso
files_to_convert_back = [
    #    ("cmd_html.h", "cmd.html"),
    # ("favicon_ico.h", "favicon.ico"),
    # ("index_html.h", "index.html"),
    # ("info_html.h", "info.html"),
    # ("ota_html.h", "ota.html"),
    # ("script_js.h", "script.js"),
    # ("serial_html.h", "serial.html"),
    # ("sha_js.h", "sha.js"),
    # ("style_css.h", "style.css"),
]

for array_file_name, output_file_name in files_to_convert_back:
    convert_from_cpp_array(array_file_name, output_file_name)
