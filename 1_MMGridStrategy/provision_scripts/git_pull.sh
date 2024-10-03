REPO_DIR="$1"
GIT_REPO_URL="$2"
GIT_BRANCH="$3"

# Default to 'main' if no branch is provided
if [ -z "$GIT_BRANCH" ]; then
    GIT_BRANCH="main"
fi

if [ -d "$REPO_DIR" ]; then
    # Change to the repository directory
    cd "$REPO_DIR" || exit

    # Fetch the latest changes from the remote repository and reset hard
    git fetch origin
    git reset --hard "origin/$GIT_BRANCH"

    # Return to the previous directory (if necessary)
    cd - || exit
    echo "We pulled the repo."
else
    # Clone the repository
    git clone -b "$GIT_BRANCH" "$GIT_REPO_URL" "$REPO_DIR"
    echo "We created the repo."
fi