import os, sys

def convert_and_inject(input_file, lua_file):
    if not os.path.exists(input_file):
        print(f"Error: No se encuentra {input_file}")
        return

    # 1. Leer binario y convertir a HEX
    with open(input_file, "rb") as f:
        bin_data = f.read()
    hex_spaced = " ".join([f"{b:02X}" for b in bin_data])
    
    if not os.path.exists(lua_file):
        print(f"Error: No se encuentra {lua_file}")
        return
        
    with open(lua_file, "r") as f:
        lines = f.readlines()

    # 2. Buscar y reemplazar la linea de la variable sc
    found = False
    with open(lua_file, "w") as f:
        for line in lines:
            if line.startswith('local sc = "'):
                f.write(f'local sc = "{hex_spaced}"\n')
                found = True
            else:
                f.write(line)

    if found:
        print(f"OK: {input_file} ({len(bin_data)} bytes) inyectado en {lua_file}")
    else:
        print(f"ERROR: No se pudo localizar 'local sc =' en {lua_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        target = "master_dump.bin"
        if not os.path.exists(target):
            target = "symbols.bin"
        if not os.path.exists(target):
            target = "explorer.bin"
        convert_and_inject(target, "nes.lua")
    else:
        convert_and_inject(sys.argv[1], sys.argv[2])
