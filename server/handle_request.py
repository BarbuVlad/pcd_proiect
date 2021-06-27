import getopt
import sys
import time

argv = sys.argv[1:]

try:
    opts, args = getopt.getopt(argv, "f:u:",)
except:
    print("Error")

for opt, arg in opts:
    if opt in ['-f']:
        flag = arg
    elif opt in ['-u']:
        user = arg

# flag = sys.argv[1]
# user = sys.argv[2]
def banUser():
    user_to_ban = ""

    try:
        with open("users.txt", "r") as f:
            users_file_content = f.readlines()
            f.close()

        with open("users.txt", "w") as f:
            for current_user in users_file_content:
                if current_user.strip("\n").split(" ")[0] == user:
                    user_to_ban = user
                    continue  # in cazul in care a gasit in lista cu users.txt user-ul primit drept input pentru a fi banat este eliminat din lista
                f.write(current_user)
                print(current_user.strip("\n").split(" ")[0])
            f.close()
        
        if len(user_to_ban) > 1:
            with open("banned_users.txt", "r") as f:
                all_banned_users = f.readlines()
                f.close()

            already_banned = False
            for banned_user in all_banned_users:
                if banned_user[:-1] == user_to_ban:
                    already_banned = True
                    break

            if already_banned == False:
                with open("banned_users.txt", "a") as f:
                    f.write(user_to_ban + "\n")
                    f.close()
                
    except:
        print("Unexpecter error in banUser")


if flag == "ban":
    banUser()
