Fixes #\<GitHub-issue-number\>.

Make sure all boxes are checked (add x inside the brackets) when you submit your contribution, remove this sentence before doing so.

- [ ] This PR is compliant with the contributing guidelines (if not, please describe why): code is fully documented and if possible a .md file is made.
- [ ] The PR text includes a **detailed explanation** (more than 50 chars)
- [ ] I have thoroughly tested my contribution.

\<Description of and rational behind this PR\>

**In case of a PR concerning the Mooltipass extension:**  
The following test procedure should be done, in the following order.  
1) Recall functionality test (Mooltipass unlocked):  
- visit a website you have credentials for  
- make sure you get prompted by your mooltipass  
- accept the credentials sending request  
- make sure you are logged in  
2) Storage functionality test (Mooltipass unlocked):  
- visit a website you don't have credentials for  
- fill the username and password field, submit  
- make sure the mooltipass prompts you for password storage **once**  
- accept the credentials storage request  
- run test 1) to make sure credentials are correctly stored  
3) Cancel functionality test (Mooltipass unlocked):  
- visit a website you have credentials for  
- make sure you get prompted by your mooltipass   
- do not accept the credentials sending request, close the tab  
- make sure the prompt gets removed on the mooltipass  
4) Credentials requests queue test (Mooltipass **locked**)  
- visit a website you have credentials for  
- unlock your mooltipass  
- make sure you get prompted by your mooltipass   
5) Credentials requests queue test (Mooltipass unlocked)  
- visit a website A you have credentials for  
- make sure you get prompted by your mooltipass   
- do not accept the credentials sending request  
- visit a website B you have credentials for  
- close the tab containing the website A  
- make sure the first prompt is cancelled on the mooltipass  
- make sure another prompt for website B is displayed on the mooltipass  