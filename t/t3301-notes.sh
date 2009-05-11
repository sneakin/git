#!/bin/sh
#
# Copyright (c) 2007 Johannes E. Schindelin
#

test_description='Test commit notes'

. ./test-lib.sh

cat > fake_editor.sh << \EOF
echo "$MSG" > "$1"
echo "$MSG" >& 2
EOF
chmod a+x fake_editor.sh
VISUAL=./fake_editor.sh
export VISUAL

test_expect_success 'cannot annotate non-existing HEAD' '
	(MSG=3 && export MSG && test_must_fail git notes edit)
'

test_expect_success setup '
	: > a1 &&
	git add a1 &&
	test_tick &&
	git commit -m 1st &&
	: > a2 &&
	git add a2 &&
	test_tick &&
	git commit -m 2nd
'

test_expect_success 'need valid notes ref' '
	(MSG=1 GIT_NOTES_REF=/ && export MSG GIT_NOTES_REF &&
	 test_must_fail git notes edit) &&
	(MSG=2 GIT_NOTES_REF=/ && export MSG GIT_NOTES_REF &&
	 test_must_fail git notes show)
'

test_expect_success 'refusing to edit in refs/heads/' '
	(MSG=1 GIT_NOTES_REF=refs/heads/bogus &&
	 export MSG GIT_NOTES_REF &&
	 test_must_fail git notes edit)
'

test_expect_success 'refusing to edit in refs/remotes/' '
	(MSG=1 GIT_NOTES_REF=refs/remotes/bogus &&
	 export MSG GIT_NOTES_REF &&
	 test_must_fail git notes edit)
'

# 1 indicates caught gracefully by die, 128 means git-show barked
test_expect_success 'handle empty notes gracefully' '
	git notes show ; test 1 = $?
'

test_expect_success 'create notes' '
	git config core.notesRef refs/notes/commits &&
	MSG=b1 git notes edit &&
	test ! -f .git/new-notes &&
	test 1 = $(git ls-tree refs/notes/commits | wc -l) &&
	test b1 = $(git notes show) &&
	git show HEAD^ &&
	test_must_fail git notes show HEAD^
'

cat > expect << EOF
commit 268048bfb8a1fb38e703baceb8ab235421bf80c5
Author: A U Thor <author@example.com>
Date:   Thu Apr 7 15:14:13 2005 -0700

    2nd

Notes:
    b1
EOF

test_expect_success 'show notes' '
	! (git cat-file commit HEAD | grep b1) &&
	git log -1 > output &&
	test_cmp expect output
'
test_expect_success 'create multi-line notes (setup)' '
	: > a3 &&
	git add a3 &&
	test_tick &&
	git commit -m 3rd &&
	MSG="b3
c3c3c3c3
d3d3d3" git notes edit
'

cat > expect-multiline << EOF
commit 1584215f1d29c65e99c6c6848626553fdd07fd75
Author: A U Thor <author@example.com>
Date:   Thu Apr 7 15:15:13 2005 -0700

    3rd

Notes:
    b3
    c3c3c3c3
    d3d3d3
EOF

printf "\n" >> expect-multiline
cat expect >> expect-multiline

test_expect_success 'show multi-line notes' '
	git log -2 > output &&
	test_cmp expect-multiline output
'

test_done
