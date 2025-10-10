# 🚀 Build and Release Process

## 🔨 Build Process
1. **Trigger:** Commit to `Feature/Issue branch attached to a Merge Request` or `master`
2. **Pipeline Stages:**
   - build_clang → build_gcc → format → lint → test  
- Build stages(build_clang and build_gcc) run independently
- Test stages (format, lint, test) run independently but runs after successful build_gcc run

---

## 📦 Release Process
1. Merge `Feature/Issue branch` → `master`
3. Publish:
   - GitLab: Internal release packages  
   - GitHub: Release and Update version (`vMAJOR.MINOR.PATCH`)

---

## 🔖 Versioning
- **Semantic Versioning:**
  - MAJOR = breaking changes
  - MINOR = new features, backwards-compatible
  - PATCH = fixes

---

## 🧩 Automation
- GitLab CI jobs for builds & test 

---

## 🛡 Quality Gates Before Release
- All tests passing
- Reviewer Approval  

---

## 📊 Post-Release
- Monitor issues for regressions an d necessary feature changes
