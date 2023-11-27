# repl.py - Simple repl module

def rep(getline, interpreter, prompt = ""):
    """Invoke the function getline that returns a string
    and consider this string as a line to interpret: if the
    line contains a backslash, invoke getline again until
    a line with no backslash is found. an empty line is
    skipped. If a not zero result is returned, it means the
    command "bye" has been typed."""
    if prompt != "": print(prompt, end=":")
    line = getline()
    s = ""
    while (i := line.find("\\")) != -1:
        s += line[:i] + " "
        if prompt != "": print(prompt, end="|")
        line = getline()
    s += line
    s = s.strip()
    if s == "bye": return 0
    elif s[:5] == "batch":
        filename = s[5:].lstrip()
        def readline():
            s = f.readline()
            return "bye" if s == "" else s
        try:
            with open(filename) as f:
                while rep(readline, interpreter, ""):
                    pass
        except FileExistsError as ex:
            print(f"Problem with file {filename}: {ex}")
    elif s != "":
        y = interpreter(s)
        #if y != None: print(y)
    return 1

def repl(interpreter, prompt = ""):
    print()
    print("Type: 'bye' to leave, 'batch FILENAME' to process a file")
    print("Lines starting with backslash or empty are skipped")
    print("A line ending with backslash is joined to the following one")
    print()
    while rep(input, interpreter, prompt):
        pass
    print("Goodbye.")
