REPO_DIR="$1"
GIT_REPO_URL="$2"
GIT_BRANCH="$3"

# Default to 'main' if no branch is provided
if [ -z "$GIT_BRANCH" ]; then
    GIT_BRANCH="main"
fi

# if [ -d "$REPO_DIR" ]; then
#     cd "$REPO_DIR"
#     git fetch origin
#     git reset --hard origin/$GIT_BRANCH
#     cd ..
#     git pull
#     echo "We pulled the repo."
# else
#     git clone -b $GIT_BRANCH $GIT_REPO_URL $REPO_DIR
#     echo "We created the repo."
# fi
