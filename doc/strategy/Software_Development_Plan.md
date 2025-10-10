# 🛠 Software Development Plan (SDP)

## 📌 Project Overview
- **Project Name:** DuRTL 
- **Short Description:** DuRTL is a hardware information flow analysis tool.
- **Repository Platforms:**  
  - Internal Development: GitLab  
  - Open Source Release: GitHub  

---

## 🌱 Development Workflow
1. **Create issue or feature branch** in GitLab  
2. **Feature branch** from master
3. **Commit convention:** 
    - Add-Commit-Push with proper documentation.
    - Not applicable for changes in vcpkg sample library  

4. **Merge request / Pull request:**  
   - Successful pipeline run
   - At least 1 reviewer approval(depending on major/minor issue or feature implemented)
5. **Merge to `feature branch` → `master`** for issue/feature release  

---

## 📂 Repository Structure

- /doc
    - /doxyfile: doxygen settings for DuRTL
    - /useCases: documentation replicating the experiments/feature tests performed with DuRTL for research works
- /tests: includes sample, feature and unit tests along with inputs for the tests
    - /testfiles: includes structured **verilog design** examples that needs to be tested along with it's corresponding **testbench** and **json**
- /thirdparty: includes dependent vcpkg registry and oss-cad-suite

Details of the following components are available as part of doxyfile documentation:
- /include/ducode
- /src

---

## 📈 Versioning & Release
- **Semantic Versioning:** `MAJOR.MINOR.PATCH`
- **Tagging:** Git tags for every release  
- **GitHub Release Page:** Published for each version  

---

## 📖 Documentation Strategy
- `README.md`: Quick Strategy
- Supporting documentation related to Build, Release and Software Management plan included in /doc/strategy
- Documentation replicating the experiments/feature tests performed with DuRTL for research work in /doc/useCases

---

## ✅ Quality Gates
- Code style checks (linting)
- Unit & integration tests must pass(currently running 102 tests)
- Current Coverage: 60.92%