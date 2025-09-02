// this program reads skillid.lub and skillinfolist.lub
// and creates two text files:
//   - SKILL_id_handle.txt   (list of all IDs and handles)
//   - skillnametable.txt    (list of handles + human names if found)
//
// build:
//   g++ -std=c++14 -O2 -o skillid_parser parse_skill.cpp
//   cl /std:c++14 /O2 parse_skill.cpp /Fe:skillid_parser.exe
//
// run: ./skillid_parser
// make sure the .lub files are in the same folder

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstdlib>

using namespace std;

// ---------- utils ----------

// cut spaces/tabs/newlines from start and end of a string
static inline void trim(string& x) {
    size_t a = x.find_first_not_of(" \t\r\n");
    size_t b = x.find_last_not_of(" \t\r\n");
    if (a == string::npos) { x.clear(); return; }
    x = x.substr(a, b - a + 1);
}

// read a whole file into a string
static bool read_file(const string& path, string& out) {
    ifstream f(path.c_str(), ios::binary);
    if (!f) return false;
    ostringstream oss; oss << f.rdbuf();
    out = oss.str();
    return true;
}

// remove lua comments (--[[ ... ]] and -- to end of line)
static string strip_lua_comments(const string& s) {
    string t; t.reserve(s.size());

    // first remove multi-line comments --[[ ... ]]
    for (size_t i = 0; i < s.size(); ) {
        if (i + 3 < s.size() && s[i] == '-' && s[i + 1] == '-' && s[i + 2] == '[' && s[i + 3] == '[') {
            i += 4;
            // replace comment chars with spaces/newlines
            while (i + 1 < s.size() && !(s[i] == ']' && s[i + 1] == ']')) {
                t.push_back((s[i] == '\n') ? '\n' : ' ');
                ++i;
            }
            if (i + 1 < s.size()) { t += "  "; i += 2; }
            continue;
        }
        t.push_back(s[i++]);
    }

    // now remove single-line comments --
    string u; u.reserve(t.size());
    for (size_t i = 0; i < t.size(); ) {
        if (i + 1 < t.size() && t[i] == '-' && t[i + 1] == '-') {
            while (i < t.size() && t[i] != '\n') ++i;
            u.push_back('\n');
            if (i < t.size()) ++i;
        }
        else {
            u.push_back(t[i++]);
        }
    }
    return u;
}

// check if string is only numbers
static inline bool is_digit_string(const string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

// get the first "..." string found
static string first_quoted(const string& body) {
    string out; bool in = false;
    for (char c : body) {
        if (!in) {
            if (c == '"') { in = true; out.clear(); }
        }
        else {
            if (c == '"') return out;
            else out.push_back(c);
        }
    }
    return "";
}

// find a field like SkillName = "value" in a block
static string find_field_string(const string& body, const char* field) {
    size_t p = 0, flen = strlen(field);
    while (true) {
        p = body.find(field, p);
        if (p == string::npos) return "";

        // check boundaries so it’s a real field
        bool left_ok = (p == 0) || !isalnum(static_cast<unsigned char>(body[p - 1]));
        bool right_ok = (p + flen >= body.size()) ||
            !isalnum(static_cast<unsigned char>(body[p + flen]));
        if (!left_ok || !right_ok) { p += flen; continue; }

        // find the = and the quotes after it
        size_t eq = body.find('=', p + flen);
        if (eq == string::npos) return "";
        size_t q1 = body.find('"', eq + 1);
        if (q1 == string::npos) { p = eq + 1; continue; }
        size_t q2 = body.find('"', q1 + 1);
        if (q2 == string::npos) return "";
        return body.substr(q1 + 1, q2 - (q1 + 1));
    }
}

// ---------- parsing ----------

// parse SKID = { NAME = VALUE, ... } block
static size_t parse_skid_block(const string& text,
    unordered_map<string, long long>& handle_to_id,
    unordered_map<long long, string>& id_to_handle)
{
    size_t added = 0;

    // find first {...} block
    size_t lb = text.find('{');
    if (lb == string::npos) return 0;

    int depth = 0;
    size_t rb = string::npos;
    for (size_t i = lb; i < text.size(); ++i) {
        if (text[i] == '{') ++depth;
        else if (text[i] == '}') { if (--depth == 0) { rb = i; break; } }
    }
    if (rb == string::npos) return 0;

    string body = text.substr(lb + 1, rb - lb - 1);

    // split by commas
    vector<string> entries;
    string cur; cur.reserve(64);
    for (char c : body) {
        if (c == ',') {
            string e = cur; trim(e);
            if (!e.empty()) entries.push_back(e);
            cur.clear();
        }
        else cur.push_back(c);
    }
    trim(cur); if (!cur.empty()) entries.push_back(cur);

    // process each entry NAME = VALUE
    for (string e : entries) {
        size_t eq = e.find('=');
        if (eq == string::npos) continue;
        string name = e.substr(0, eq);
        string val = e.substr(eq + 1);
        trim(name); trim(val);
        if (name.empty() || val.empty()) continue;

        // remove ; if any
        size_t semi = val.find(';');
        if (semi != string::npos) val.erase(semi);
        trim(val);

        // convert to number
        char* endp = NULL;
        long long id = strtoll(val.c_str(), &endp, 0);
        if (endp == val.c_str()) continue;

        handle_to_id[name] = id;
        id_to_handle[id] = name;
        ++added;
    }
    return added;
}

// read skillid.lub and build maps
static bool load_ids_from_lub(const string& path,
    unordered_map<string, long long>& handle_to_id,
    unordered_map<long long, string>& id_to_handle)
{
    string raw;
    if (!read_file(path, raw)) return false;
    string t = strip_lua_comments(raw);

    // find "SKID"
    size_t p = t.find("SKID");
    if (p == string::npos) return false;
    size_t brace = t.find('{', p);
    if (brace == string::npos) return false;

    string block = t.substr(brace);
    return parse_skid_block(block, handle_to_id, id_to_handle) > 0;
}

// read skillinfolist.lub and get id -> SkillName
static bool load_names_from_skillinfolist_lub(
    const string& path,
    const unordered_map<string, long long>& handle_to_id,
    unordered_map<long long, string>& id_to_name)
{
    string raw;
    if (!read_file(path, raw)) return false;
    string t = strip_lua_comments(raw);

    size_t i = 0;
    while (true) {
        // find [ ... ]
        size_t lb = t.find('[', i);
        if (lb == string::npos) break;
        size_t rb = t.find(']', lb + 1);
        if (rb == string::npos) break;

        string key = t.substr(lb + 1, rb - lb - 1);
        trim(key);

        // find { ... }
        size_t eq = t.find('=', rb + 1);
        if (eq == string::npos) { i = rb + 1; continue; }
        size_t ob = t.find('{', eq + 1);
        if (ob == string::npos) { i = rb + 1; continue; }

        int depth = 0; size_t cb = string::npos;
        for (size_t j = ob; j < t.size(); ++j) {
            if (t[j] == '{') ++depth;
            else if (t[j] == '}') { if (--depth == 0) { cb = j; break; } }
        }
        if (cb == string::npos) break;

        string body = t.substr(ob + 1, cb - ob - 1);

        // figure out skill_id
        long long skill_id = -1;
        if (is_digit_string(key)) {
            skill_id = strtoll(key.c_str(), NULL, 10);
        }
        else {
            // key like SKID.NV_BASIC or NV_BASIC
            size_t dot = key.rfind('.');
            string handle = (dot == string::npos) ? key : key.substr(dot + 1);
            trim(handle);
            handle.erase(remove_if(handle.begin(), handle.end(),
                [](char c) { return isspace((unsigned char)c); }), handle.end());
            if (!handle.empty()) {
                auto it = handle_to_id.find(handle);
                if (it != handle_to_id.end()) skill_id = it->second;
            }
        }

        // get SkillName
        string skill_name = find_field_string(body, "SkillName");
        if (!skill_name.empty() && skill_id >= 0) {
            id_to_name[skill_id] = skill_name;
        }

        i = cb + 1;
    }

    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // load ids from skillid.lub
    unordered_map<string, long long> handle_to_id;
    unordered_map<long long, string> id_to_handle;
    if (!load_ids_from_lub("skillid.lub", handle_to_id, id_to_handle)) {
        cerr << "Error: couldn't read/parse skillid.lub (SKID block missing?).\n";
        return 1;
    }

    // make sorted list by ID
    vector<pair<long long, string>> id_handle_vec;
    id_handle_vec.reserve(id_to_handle.size());
    for (auto& kv : id_to_handle) id_handle_vec.push_back(kv);
    sort(id_handle_vec.begin(), id_handle_vec.end(),
        [](const pair<long long, string>& a, const pair<long long, string>& b) { return a.first < b.first; });

    // write SKILL_id_handle.txt
    {
        ofstream out("SKILL_id_handle.txt");
        if (!out) { cerr << "Cannot write SKILL_id_handle.txt\n"; return 1; }
        for (auto& p : id_handle_vec) {
            out << p.first << ' ' << p.second << '\n';
        }
        out.close();
        cout << "Wrote " << id_handle_vec.size() << " lines to SKILL_id_handle.txt\n";
    }

    // load skill names from skillinfolist.lub
    unordered_map<long long, string> id_to_skillname;
    if (!load_names_from_skillinfolist_lub("skillinfolist.lub", handle_to_id, id_to_skillname)) {
        cerr << "Error: couldn't read skillinfolist.lub — skillnametable.txt will be empty.\n";
    }

    // write skillnametable.txt
    {
        ofstream out("skillnametable.txt");
        if (!out) { cerr << "Cannot write skillnametable.txt\n"; return 1; }
        size_t wrote = 0;
        for (auto& p : id_handle_vec) {
            long long id = p.first;
            const string& handle = p.second;
            auto it = id_to_skillname.find(id);
            if (it == id_to_skillname.end()) continue; // skip if no name
            out << handle << "#" << it->second << "#\n";
            ++wrote;
        }
        out.close();
        cout << "Wrote " << wrote << " lines to skillnametable.txt\n";
    }

    return 0;
}
