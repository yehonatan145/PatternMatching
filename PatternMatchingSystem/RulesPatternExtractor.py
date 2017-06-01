"""
script to extract patterns from given rules files
args:
1:  ids - the ids/ips with the rules to extract
    "snort" - snort/suricata rules
    "clamav" - clamAV rules (should be unpacked cvd)
2: dirpath - the path of the directory to search rules files in
3: output_file - the name of the output file to put extracted rules in
"""
import os
import string
import sys

def next_not_space(txt, pos):
    """ find next position in text which is not space """
    try:
        return pos + next(i for i,j in enumerate(txt[pos:]) if j not in string.whitespace)
    except StopIteration:
        return -1

def snort_extract_patterns_from_line(line):
    """ return list of patterns from the given rule line
        (in snort, the pattern is written as ' content:"pattern" ')
    """
    res = []
    pos = 0
    while pos != -1:
        pos = line.find("content", pos)
        if pos == -1:
            continue
        pos += len("content")
        pos = next_not_space(line, pos)
        if pos == -1 or line[pos] != ':':
            continue
        pos = next_not_space(line, pos + 1)
        if pos == -1 or line[pos] != '"':
            continue
        next_pos = pos
        while True:
            next_pos = line.find('"',next_pos+1)
            if next_pos == -1 or line[next_pos-1] != '\\':
                break
        if next_pos == -1:
            break
        res.append(line[pos+1:next_pos])
    return res

def snort_extract_patterns_from_file(filename):
    """ return list of patterns from the file with that file name """
    res = []
    f = open(filename, 'r')
    for line in f:
        if len(line) >= 2:
            while line[-2] == '\\':
                line = line[:-2] + f.readline()
        res += snort_extract_patterns_from_line(line)
    f.close()
    return res

def snort_write_patterns_from_dir(dirpath,output_file_name):
    """ write to output file all patterns from all rules files
        in the specified directory
    """
    n_rules = 0
    output_file = open(output_file_name, 'w')
    for path, dirs, files in os.walk(dirpath):
        for filename in files:
            if filename.endswith('.rules'):
                fullpath = os.path.join(path, filename)
                print("parsing file: " + fullpath)
                pats = snort_extract_patterns_from_file(fullpath)
                n_rules += len(pats)
                for pat in pats:
                    output_file.write(pat + '\n')
    print("done. " + str(n_rules) + " rules were parsed to file " + output_file_name)
    output_file.close()

def find_nth(txt, pat, n):
    """ find the nth occurance of 'pat' inside 'txt' """
    pos = txt.find(pat)
    for i in range(n-1):
        if pos == -1:
            return -1
        pos = txt.find(pat, pos + 1)
    return pos

hexa_chars = set('0123456789ABCDEF')

def clamav_extract_pattern_from_line(line):
    """ return the pattern write in the line """
    start_pos = find_nth(line, ':', 3) + 1
    end_pos = line.find(':', start_pos)
    if end_pos == -1:
        end_pos = len(line) - 1 # -1 for the \n in the end
    txt = line[start_pos:end_pos].upper()
    if any((c not in hexa_chars) for c in txt) or (len(txt) & 1):
        return ""
    return '|' + txt + '|'

def clamav_extract_patterns_from_ndb_file(filename):
    """ return list of patterns from a ndb file """
    res = []
    f = open(filename, 'r')
    for line in f:
        pat = clamav_extract_pattern_from_line(line)
        if pat != "":
            res.append(pat)
    f.close()
    return res

def clamav_write_patterns_from_dir(dirpath, output_file_name):
    """ write all patterns in directory to output file """
    n_rules = 0
    output_file = open(output_file_name, 'w')
    for path, dirs, files in os.walk(dirpath):
        for filename in files:
            if filename.endswith('.ndb'):
                fullpath = os.path.join(path, filename)
                print("parsing file: " + fullpath)
                pats = clamav_extract_patterns_from_ndb_file(fullpath)
                n_rules += len(pats)
                for pat in pats:
                    output_file.write(pat + '\n')
    print("done. " + str(n_rules) + " rules were parsed to file " + output_file_name)
    output_file.close()

usage_str = """Usage: RulesPatternExtractor <ids> <dir> <output>\n\n""" \
            """ids is the ids/ips that specify the rule format (see below)\n""" \
            """dir is the directory to search the rules files in\n""" \
            """output is the name of the of the file to output the rules\n""" \
            """\nids supported:\n""" \
            """'snort': snort ids (also fit for Suricata - using the same rules format)\n"""\
            """         search for files with .rules extension\n"""\
            """'clamav': clamAV ids\n"""\
            """          search for .ndb files (unzipped from cvd files)\n"""

if __name__ == "__main__":
    if (len(sys.argv) != 4):
        print(usage_str)
        exit()
    ids = sys.argv[1]
    dirpath = sys.argv[2]
    output_file_name = sys.argv[3]
    if ids == "snort":
        snort_write_patterns_from_dir(dirpath, output_file_name)
    elif ids == "clamav":
        clamav_write_patterns_from_dir(dirpath, output_file_name)
    else:
        print(usage_str)
