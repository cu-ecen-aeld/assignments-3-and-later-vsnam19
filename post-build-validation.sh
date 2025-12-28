#!/bin/bash
# Post-build validation script for Assignment 4
# Run this after the build completes successfully

set -e

REPO_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$REPO_ROOT"

echo "========================================="
echo "Assignment 4 Post-Build Validation"
echo "========================================="
echo ""

# Check if build completed
if [ ! -f buildroot/output/images/Image ]; then
    echo "ERROR: Build has not completed yet. Please wait for ./build.sh to finish."
    exit 1
fi

echo "✓ Build completed successfully!"
echo ""

# Instructions for manual validation
echo "Manual Validation Steps:"
echo "========================"
echo ""
echo "1. Start QEMU:"
echo "   ./runqemu.sh"
echo ""
echo "2. In QEMU, login with root/root and test:"
echo "   # finder-test.sh"
echo "   # cat /tmp/assignment4-result.txt"
echo "   # ls /var/log/messages"
echo ""
echo "3. In another terminal, test SSH:"
echo "   ssh -p 10022 root@localhost"
echo "   (password: root)"
echo ""
echo "4. Transfer the result file:"
echo "   scp -P 10022 root@localhost:/tmp/assignment4-result.txt assignments/assignment4/"
echo ""
echo "5. Run full test (requires sshpass):"
echo "   sudo apt-get install sshpass  # if not installed"
echo "   ./full-test.sh"
echo ""
echo "6. Commit the result file:"
echo "   git add assignments/assignment4/assignment4-result.txt"
echo "   git commit -m 'Add assignment4 result file'"
echo ""
echo "7. Tag the repository:"
echo "   git tag assignment-4-complete"
echo "   git push origin main --tags"
echo ""
echo "========================================="
echo "Build artifacts are in:"
echo "  buildroot/output/images/"
echo "========================================="
