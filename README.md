This tool parses Ragnarok Online style `.lub` files 
(skillid.lub and skillinfolist.lub) and generates 
text files mapping skill IDs, handles, and names.

-------------------------------------------
 FEATURES
-------------------------------------------
- Reads `skillid.lub` to build ID <-> Handle list
- Reads `skillinfolist.lub` to extract SkillName
- Creates two output files:
  1) SKILL_id_handle.txt   -> "<ID> <HANDLE>"
  2) skillnametable.txt    -> "HANDLE#SkillName#"
- Skips comments in `.lub` files automatically
- Works on Windows, Linux, and macOS

-------------------------------------------
 USAGE
-------------------------------------------
1. Put these files in the same folder as the program:
   - skillid.lub         (REQUIRED)
   - skillinfolist.lub   (REQUIRED for name table)

2. Run the program:
   ./skillid_parser

3. Outputs will be created in the same folder:
   - SKILL_id_handle.txt
   - skillnametable.txt

-------------------------------------------
 NOTES
-------------------------------------------
- Requires `.lub` files already decompiled to text.
- If skillinfolist.lub is missing, skillnametable.txt 
  will be empty.
- Only extracts the `SkillName` field for now.

-------------------------------------------
 AUTHOR
-------------------------------------------
AndenDev 
