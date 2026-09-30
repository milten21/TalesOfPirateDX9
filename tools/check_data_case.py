#!/usr/bin/env python3
import argparse
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SERVERS = ["AccountServer", "GateServer", "GroupServer", "GameServer"]
CODE_DIRS = [
    "sources/Server",
    "sources/Libraries/common",
    "sources/Libraries/Util",
    "sources/Libraries/Server",
    "sources/Libraries/InfoNet",
    "sources/Libraries/Status",
    "sources/Libraries/ICUHelper",
]
CODE_EXT = (".cpp", ".h", ".hpp", ".c", ".inl")
FILE_EXT = ("lua", "txt", "bin", "blk", "atr", "cfg", "loc", "res", "dat", "ini", "csv")
MAP_LOADER_SOURCE = "sources/Server/GameServer/src/GameApp.cpp"
MAP_ENTITY_SOURCE = "sources/Server/GameServer/src/SubMap.cpp"
MAP_FIELD_EXTENSIONS = {
    "m_szSectionFile": ".atr",
    "m_szMonsterSpawnFile": ".txt",
    "m_szNpcSpawnFile": ".txt",
    "m_szMapSwitchFile": ".txt",
}
IGNORED_ROOTS = ("log", "logfile")

STRING_RE = re.compile(r'"((?:[^"\\\n]|\\.)*)"|\'((?:[^\'\\\n]|\\.)*)\'')
PATH_RE = re.compile(r"^(?:\.{1,2}/)*[A-Za-z0-9_.\- ]+(?:/[A-Za-z0-9_.\- ]+)*/?$")
STEM_RES = [
    re.compile(r"\w+\s*\[[^\]]*\]\s*=\s*\"([A-Za-z_]\w*)\""),
    re.compile(r"Load\s*\(\s*\"([A-Za-z_]\w*)\"\s*,\s*\"([A-Za-z_]\w*)\""),
]
MAP_START_RE = re.compile(
    r"strcpy\(m_MapList\[i\]->(m_sz\w+),\s*m_MapList\[i\]->GetName\(\)\)"
)
MAP_PART_RE = re.compile(
    r"strcat\(m_MapList\[i\]->(m_sz\w+),\s*(m_MapList\[i\]->GetName\(\)|\"([^\"]*)\")\)"
)
MAP_ENTITY_RE = re.compile(r"sprintf\(\s*szMapEntity,\s*\"%s/%s%s\",\s*GetName\(\),\s*GetName\(\),\s*\"([^\"]+)\"")
CFG_TOKEN_RE = re.compile(r"[A-Za-z0-9_./\\\-]+\.(?:%s)\b" % "|".join(FILE_EXT))
PM_RE = re.compile(r"\bPM\s*\(\s*[\"']([A-Za-z0-9_]+)[\"']\s*\)")
SET_CUR_MAP_RE = re.compile(r"\bSetCurMap\s*\(\s*[\"']([A-Za-z0-9_]+)[\"']\s*\)")

_listing = {}


def listing(path):
    cached = _listing.get(path)
    if cached is None:
        try:
            names = os.listdir(path)
        except OSError:
            names = []
        cached = {}
        for name in names:
            cached.setdefault(name.lower(), []).append(name)
        _listing[path] = cached
    return cached


def resolve(base, ref):
    current = base
    inexact = False
    for part in ref.replace("\\", "/").split("/"):
        if part in ("", "."):
            continue
        if part == "..":
            current = os.path.dirname(current)
            continue
        if not os.path.isdir(current):
            return None, None
        candidates = listing(current).get(part.lower())
        if not candidates:
            return None, None
        if part in candidates:
            current = os.path.join(current, part)
        else:
            inexact = True
            current = os.path.join(current, candidates[0])
    return ("icase" if inexact else "exact"), current


def rel(path):
    return os.path.relpath(path, ROOT).replace("\\", "/")


def read(path):
    with open(path, "rb") as handle:
        return handle.read().decode("latin-1")


def walk(top, suffixes):
    for directory, _, files in os.walk(top):
        for name in files:
            if name.lower().endswith(suffixes):
                yield os.path.join(directory, name)


class Checker:
    def __init__(self):
        self.mismatches = {server: {} for server in SERVERS}
        self.unresolved = {server: set() for server in SERVERS}
        self.checked = {server: 0 for server in SERVERS}

    def server_dir(self, server):
        return os.path.join(ROOT, "server", server)

    def report(self, server, source, ref, actual):
        key = (rel(actual), ref)
        self.mismatches[server].setdefault(key, set()).add(source)

    def check(self, server, source, ref, bases, allow_bare=False):
        first = ref.replace("\\", "/").lstrip("./").split("/")[0].lower()
        if first in IGNORED_ROOTS:
            return
        self.checked[server] += 1
        best = None
        for base in bases:
            status, actual = resolve(base, ref)
            if status == "exact":
                return
            if status == "icase" and best is None:
                best = actual
        if best is None and allow_bare and "/" not in ref.replace("\\", "/"):
            best = self.find_bare(server, ref)
            if best == "exact":
                return
        if best:
            self.report(server, source, ref, best)
        else:
            self.unresolved[server].add((source, ref))

    def find_bare(self, server, name):
        inexact = None
        for directory, _, files in os.walk(self.server_dir(server)):
            for candidate in files:
                if candidate == name:
                    return "exact"
                if candidate.lower() == name.lower() and inexact is None:
                    inexact = os.path.join(directory, candidate)
        return inexact

    def bases(self, server, extra=()):
        root = self.server_dir(server)
        return list(extra) + [os.path.join(root, "resource"), root]

    def literals(self, text):
        for match in STRING_RE.finditer(text):
            value = match.group(1) if match.group(1) is not None else match.group(2)
            yield match.start(), value.replace("\\\\", "/")

    def is_path_ref(self, value):
        if not PATH_RE.match(value):
            return False
        if value.endswith("/"):
            return len(value.strip("./")) > 0
        stem, dot, ext = value.rpartition(".")
        return bool(dot) and ext.lower() in FILE_EXT and bool(stem.strip("./"))

    def scan_literals(self, servers, source, text, extra_bases=()):
        for offset, value in self.literals(text):
            if self.is_path_ref(value):
                line = text.count("\n", 0, offset) + 1
                for server in servers:
                    self.check(
                        server,
                        "%s:%d" % (rel(source), line),
                        value,
                        self.bases(server, extra_bases),
                        allow_bare=True,
                    )

    def stems(self, server):
        resource = os.path.join(self.server_dir(server), "resource")
        found = {}
        for lower, names in listing(resource).items():
            for name in names:
                stem, dot, ext = name.rpartition(".")
                if dot and ext.lower() == "txt":
                    found.setdefault(stem.lower(), []).append(name)
        return found

    def scan_stems(self, servers, source, text):
        for pattern in STEM_RES:
            for match in pattern.finditer(text):
                line = text.count("\n", 0, match.start()) + 1
                for value in match.groups():
                    for server in servers:
                        names = self.stems(server).get(value.lower())
                        if not names:
                            continue
                        self.checked[server] += 1
                        if value + ".txt" not in names:
                            resource = os.path.join(self.server_dir(server), "resource")
                            self.report(
                                server,
                                "%s:%d" % (rel(source), line),
                                value + ".txt",
                                os.path.join(resource, names[0]),
                            )

    def scan_code(self):
        for top in CODE_DIRS:
            for path in walk(os.path.join(ROOT, top), CODE_EXT):
                if "/logvwr/" in path.replace("\\", "/"):
                    continue
                text = read(path)
                servers = self.owning_servers(path)
                self.scan_literals(servers, path, text)
                self.scan_stems(servers, path, text)

    def owning_servers(self, path):
        normalized = path.replace("\\", "/")
        for server in SERVERS:
            if "/sources/Server/%s/" % server in normalized:
                return [server]
        return SERVERS

    def scan_lua(self):
        for server in SERVERS:
            root = self.server_dir(server)
            for path in walk(root, (".lua",)):
                text = read(path)
                directory = os.path.dirname(path)
                self.scan_literals([server], path, text, extra_bases=[directory])
                for match in PM_RE.finditer(text):
                    line = text.count("\n", 0, match.start()) + 1
                    self.check(
                        server,
                        "%s:%d" % (rel(path), line),
                        "script/monster/%s.lua" % match.group(1),
                        self.bases(server),
                    )
                for match in SET_CUR_MAP_RE.finditer(text):
                    line = text.count("\n", 0, match.start()) + 1
                    self.check(
                        server,
                        "%s:%d" % (rel(path), line),
                        match.group(1),
                        [os.path.join(self.server_dir("GameServer"), "resource")],
                    )

    def scan_config(self):
        map_dirs_base = os.path.join(self.server_dir("GameServer"), "resource")
        for server in SERVERS:
            root = self.server_dir(server)
            for path in walk(root, (".cfg", ".loc", ".bat")):
                section = ""
                for number, line in enumerate(read(path).splitlines(), 1):
                    source = "%s:%d" % (rel(path), number)
                    stripped = line.split("//")[0].strip()
                    header = re.match(r"^\[(.+)\]$", stripped)
                    if header:
                        section = header.group(1).strip().lower()
                        continue
                    key, eq, value = stripped.partition("=")
                    key = key.strip().lower()
                    value = value.strip()
                    if eq and value and (
                        key in ("map", "chaos_map") or section == "bird"
                    ):
                        self.check(server, source, value, [map_dirs_base])
                    if eq and key in ("res_dir", "log_dir"):
                        continue
                    for token in CFG_TOKEN_RE.findall(stripped):
                        self.check(
                            server,
                            source,
                            token,
                            self.bases(server),
                            allow_bare=True,
                        )

    def scan_tables(self):
        for server in SERVERS:
            resource = os.path.join(self.server_dir(server), "resource")
            for path in walk(resource, (".txt",)):
                for number, line in enumerate(read(path).splitlines(), 1):
                    if line.startswith("//"):
                        continue
                    for cell in line.split("\t"):
                        cell = cell.strip().replace("\\", "/")
                        if self.is_path_ref(cell) and not cell.endswith("/"):
                            self.check(
                                server,
                                "%s:%d" % (rel(path), number),
                                cell,
                                self.bases(server, [os.path.dirname(path)]),
                                allow_bare=True,
                            )

    def map_templates(self):
        fields = {}
        order = []
        for line in read(os.path.join(ROOT, MAP_LOADER_SOURCE)).splitlines():
            start = MAP_START_RE.search(line)
            if start:
                fields[start.group(1)] = ["{d}"]
                order.append(start.group(1))
                continue
            part = MAP_PART_RE.search(line)
            if part and part.group(1) in fields:
                fields[part.group(1)].append(
                    "{d}" if part.group(3) is None else part.group(3)
                )
        if not fields:
            raise SystemExit("cannot derive map file names from " + MAP_LOADER_SOURCE)
        templates = []
        for field in order:
            template = "".join(fields[field]) + MAP_FIELD_EXTENSIONS.get(field, "")
            templates.append(template)
        entity = MAP_ENTITY_RE.search(read(os.path.join(ROOT, MAP_ENTITY_SOURCE)))
        if not entity:
            raise SystemExit("cannot derive map entity name from " + MAP_ENTITY_SOURCE)
        templates.append("{d}/{d}" + entity.group(1))
        return templates

    def scan_maps(self):
        resource = os.path.join(self.server_dir("GameServer"), "resource")
        server = "GameServer"
        templates = self.map_templates()
        for name in sorted(os.listdir(resource)):
            directory = os.path.join(resource, name)
            if not os.path.isdir(directory):
                continue
            if not any(
                entry.lower().endswith(".blk") for entry in os.listdir(directory)
            ):
                continue
            for template in templates:
                ref = template.format(d=name)
                self.checked[server] += 1
                status, actual = resolve(resource, ref)
                if status == "icase":
                    self.report(server, "map " + name, ref, actual)

    def run(self):
        self.scan_code()
        self.scan_lua()
        self.scan_config()
        self.scan_tables()
        self.scan_maps()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    checker = Checker()
    checker.run()

    total = 0
    for server in SERVERS:
        found = checker.mismatches[server]
        total += len(found)
        print("%s: %d references checked, %d mismatches" % (
            server, checker.checked[server], len(found)))
        for (actual, ref), sources in sorted(found.items()):
            print("  reference '%s' -> on disk '%s'" % (ref, actual))
            for source in sorted(sources)[:3]:
                print("    from %s" % source)
            if len(sources) > 3:
                print("    and %d more" % (len(sources) - 3))
        if args.verbose:
            for source, ref in sorted(checker.unresolved[server]):
                print("  unresolved '%s' from %s" % (ref, source))
    print("total mismatches: %d" % total)
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
