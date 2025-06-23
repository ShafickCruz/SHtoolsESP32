# type: ignore

import os
import shutil

Import("env")

src_base = os.path.join(env["PROJECT_DIR"], ".pio", "libdeps", env["PIOENV"])
dst_base = os.path.join(env["PROJECT_DIR"], "lib")


def copiar_libs():
    if not os.path.exists(src_base):
        print(f"[SHtools] Pasta de origem não encontrada: {src_base}")
        return

    if not os.path.exists(dst_base):
        os.makedirs(dst_base)

    for lib in os.listdir(src_base):
        src = os.path.join(src_base, lib)
        dst = os.path.join(dst_base, lib)

        if os.path.isdir(src):
            print(f"[SHtools] Copiando {lib}...")
            shutil.copytree(src, dst, dirs_exist_ok=True)
        else:
            print(f"[SHtools] Ignorando arquivo (não é pasta): {lib}")


copiar_libs()
