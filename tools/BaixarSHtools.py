import os
import subprocess
import shutil

repo_url = "https://github.com/ShafickCruz/SHtoolsESP32.git"
repo_dir = "lib/SHtoolsESP32"
proj_lib_dir = "lib"

# Clona o repositório, se necessário
if not os.path.exists(repo_dir):
    print(f"Clonando SHtools para {repo_dir}...")
    subprocess.run(["git", "clone", "--depth=1", repo_url, repo_dir], check=True)
else:
    print("SHtools já está presente. Nenhuma ação necessária.")

# Move as libs internas de SHtoolsESP32/lib/ para lib/ do projeto, se não existirem
src_libs_dir = os.path.join(repo_dir, "lib")
if os.path.exists(src_libs_dir):
    for lib_name in os.listdir(src_libs_dir):
        lib_path = os.path.join(src_libs_dir, lib_name)
        target_path = os.path.join(proj_lib_dir, lib_name)
        if os.path.isdir(lib_path) and not os.path.exists(target_path):
            print(f"Movendo {lib_name} para {target_path}...")
            shutil.move(lib_path, target_path)
        else:
            print(f"{lib_name} já existe em {target_path}. Nenhuma ação.")
else:
    print(f"Nenhuma pasta encontrada em {src_libs_dir}")
