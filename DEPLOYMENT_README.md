# GitHub Releases Deployment Setup

This document explains how to configure automatic deployment of build artifacts to GitHub Releases using AppVeyor CI/CD.

## 🚀 Current Configuration

The `.appveyor.yml` file is configured to automatically upload build artifacts to GitHub Releases when:
- A git tag is pushed to the repository
- All builds complete successfully

### Artifacts Uploaded:
- **Ubuntu**: `.deb` packages for Linux
- **Windows**: `.exe`, `.msi`, and `.nupkg` packages

## 🔧 Setup Instructions

### 1. Generate GitHub Personal Access Token

1. Go to [GitHub Settings → Developer settings → Personal access tokens](https://github.com/settings/tokens)
2. Click "Generate new token (classic)"
3. Select scope: `public_repo` (or `repo` for private repos)
4. Copy the generated token

### 2. Encrypt Token in AppVeyor

#### Option A: Add encrypted environment variable (recommended):
1. Go to your [AppVeyor project settings](https://ci.appveyor.com/project/fishca/v8unpack/settings)
2. Navigate to "Environment" section
3. Add encrypted variable:
   - **Name**: `GITHUB_TOKEN` (or `auth_token`)
   - **Value**: Paste your GitHub token and click "Encrypt"

#### Option B: Encrypt token locally and update appveyor.yml:
1. Get your encrypted token value from AppVeyor (add the variable, encrypt, then copy the encrypted value)
2. Replace the placeholder in `appveyor.yml`

### 3. Update AppVeyor Configuration

In `appveyor.yml`, replace the placeholder with your encrypted token:

```yaml
auth_token:
  secure: YOUR_ENCRYPTED_TOKEN_HERE  # Replace with actual encrypted token from AppVeyor
```

**Note:** The current placeholder token `secure: 4f56e4a2-5c56-4b8a-9e8b-8b2f8e4b8f9e` will cause 401 Unauthorized errors until replaced with a real encrypted token.

## 📋 Deployment Logic

- **Trigger**: Only runs on tagged commits (`appveyor_repo_tag: true`)
- **Release Type**: Standard release (not draft or prerelease)
- **Artifact Patterns**:
  - Ubuntu: `v8unpack-*.deb`
  - Windows: `v8unpack-*.exe`, `v8unpack-*.msi`, `v8unpack-*.nupkg`

## 🏷️ Creating Releases

To create a new release:

1. Create and push a git tag:
   ```bash
   git tag v3.0.43
   git push origin v3.0.43
   ```

2. AppVeyor will automatically:
   - Build on both Ubuntu and Windows
   - Create artifacts
   - Upload to GitHub Releases

## 🔍 Validation

After deployment, verify:
- GitHub Releases page shows new release
- All platform artifacts are attached
- Release is marked as latest (adjust manually if needed)

## 🛡️ Security Notes

- Use dedicated GitHub tokens with minimal permissions
- Regularly rotate access tokens
- Never commit actual token values to repository
