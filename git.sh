COMMIT=$1
RAMA=$2
git status
git add .
git commit -m "$COMMIT"
git push origin $RAMA