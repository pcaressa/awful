# scan.py module - simple scanner

def scan(s, keywords = {}, delimiters = ""):
    """
        Scan tokens from string s and returns a list containing
        them: each item in the list is a tuple of the form:
    
        - ("NUMBER", n)
        - ("STRING", s)
        - ("ATOM", a)
        - (k, None) if k is in the keywords dictionary or list
        - (d, None) if k is a character in the delimiters string
    """
    def scan_until(s, i, d):
        """Scans a string s from character i on and stops when
        the string is ended or one of the characters in string
        d are scanned: the pair (j, t) is returned, being j the
        updated value of i and t the parsed token."""
        j = i + 1
        while j < len(s) and s[j] not in d:
            j += 1
        return j, s[i:j]

    SPACES = " \t\n\r"
    tl = []
    i = 0
    while i < len(s):
        c = s[i]
        if c in SPACES:
            i += 1
        elif c in delimiters:
            tl.append([c, c])
            i += 1
        elif c in "\"\'":
            i, c = scan_until(s, i+1, c)
            tl.append(["STRING", c])
            i += 1  # skip the final '"'
        else:
            i, c = scan_until(s, i, SPACES + delimiters)
            try:
                tl.append(["NUMBER", float(c)])
            except:
                if c in keywords:
                    tl.append([c, c])
                else:
                    tl.append(["ATOM", c])
    return tl

def expect(c, tok_expected):
    """Parses a token from c and drop it from c that is returned
    as value; the token must be of type tok_expected
    else an error is raised."""
    tok, val, c = parse(c)
    if tok != tok_expected:
        raise BaseException(f"Error: '{tok_expected}' expected")
    return c

def look_ahead(c):
    """Raise an exception if c is [], else return the first
    item tok of the first token (tok, val) in c without altering
    c."""
    if len(c) == 0:
        raise BaseException("Unexpected end of text")
    return c[0][0]

def parse(c):
    """Parse a token from c and return the triple (tok, val, c)
    where c is the new one after the token has been popped,
    tok and val are the elements of the parsed token, which is
    a pair. If c is empty an error is raised."""
    tok = look_ahead(c)
    return tok, c[0][1], c[1:]