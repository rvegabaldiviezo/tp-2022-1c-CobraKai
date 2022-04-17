MENSAJE=$1
RAMA=$2
RAMADEV=develop
git status
git add .
git commit -m "$MENSAJE"
git checkout $RAMADEV  
git pull origin $RAMADEV
git checkout $RAMA
git rebase $RAMADEV
git checkout $RAMADEV
git merge $RAMA
git push origin $RAMADEV