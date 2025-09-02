===========================================
 SkillID Parser
===========================================

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
 OUTPUT FORMAT
-------------------------------------------
SKILL_id_handle.txt
    Each line:  <ID> <HANDLE>
    Example:
        1 NV_BASIC
        89 MG_SAFETYWALL

skillnametable.txt
    Each line:  HANDLE#SkillName#
    Example:
        NV_BASIC#Basic Skill#
        MG_SAFETYWALL#Safety Wall#

-------------------------------------------
 EXAMPLE WORKFLOW
-------------------------------------------
Input: skillid.lub
    SKID = {
      NV_BASIC = 1,
      MG_SAFETYWALL = 89,
    }

Input: skillinfolist.lub
    SKILL_INFO_LIST = {
      [SKID.NV_BASIC] = { SkillName = "Basic Skill" },
      [89]            = { SkillName = "Safety Wall" }
    }

Output: SKILL_id_handle.txt
    1 NV_BASIC
    89 MG_SAFETYWALL

Output: skillnametable.txt
    NV_BASIC#Basic Skill#
    MG_SAFETYWALL#Safety Wall#

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
