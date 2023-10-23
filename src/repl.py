# repl.py - Simple repl module

def repl(interpreter, prompt = ""):
    print("[type: 'bye' to leave, 'batch FILENAME' to process a file]")
    print()
    while (p := input(prompt + ": ")) != "bye":
        p = p.strip()
        if p == "":
            continue
        # Join lines ending with backslash
        while len(p) > 0 and p[-1] =='\\':
            q = input(prompt + "| ").strip()
            p += q
        if p[:6] == "batch ":
            try:
                filename = p[6:].strip()
                with open(filename) as f:
                    q = ""
                    for line in f:
                        stripped = line.strip()
                        if stripped == "":
                            # evaluates the string s
                            print(interpreter(q))
                            q = ""
                        elif stripped[0] != ";":
                            q += stripped + " "
                    print(interpreter(q))
            except FileExistsError as ex:
                print(f"Problem with file {filename}: {ex}")
        else:
            y = interpreter(p)
            if y != None:
                print(y)
    print("Goodbye.")