# repl.py - Simple repl module

def repl(interpreter, prompt = ""):
    print()
    print("Type: 'bye' to leave, 'batch FILENAME' to process a file")
    print("Lines starting with backslash or empty are skipped")
    print("A line ending with backslash is joined to the following one")
    print()
    while (p := input(prompt + ": ")) != "bye":
        p = p.strip()
        # Skip empty lines or lines starting with backslash
        if p == "" or (len(p) > 0 and p[0] == "\\"):
            continue
        # Join lines ending with backslash
        while len(p) > 0 and p[-1] =='\\':
            q = input(prompt + "| ").strip()
            p = p[:-1] + q  # strip also the '\\' on the right
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
