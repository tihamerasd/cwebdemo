#!/bin/bash
#
#
#

## to generate ASCII codes, vica-versa
chr() {
    printf \\$(printf '%03o' $1)
}

ord() {
    printf '%d' "'$1'"
}

## exercice OK
std_ready(){

    ((exercise_ok++))
    CODE="$CODE 1"

    case $SECTION in
        1)
            ((sub_1++))
            ((all_1++))
          ;;
        2)
            ((sub_2++))
            ((all_2++))
          ;;
        3)
            ((sub_3++))
            ((all_3++))
          ;;
    esac

    echo "                                                   +1"
    echo
}

## exercice FAILED
std_failed(){

    ((exercise_failed++))
    CODE="$CODE 0"

    case $SECTION in

        1)
            ((all_1++))
          ;;
        2)
            ((all_2++))
          ;;
        3)
            ((all_3++))
          ;;
    esac

    echo "                                                    0"
    echo
}

## optional exercice OK
opt_ready(){
    ((exercise_optional++))
    CODE="$CODE 1"
    echo "                                                   +1"
    echo
}

opt_failed(){
    ((exercise_failed++))
    CODE="$CODE 0"
    echo "                                                    0"
    echo
}

############################################################
############################################################

echo
echo
echo "   Információs rendszerek üzemeltetése"
echo "         -- Linux admin labor --"
echo "           --  jegyzőkönyv --"
echo "            --  v. 9.1.14 --"
echo
echo

var1=`whoami`
if [ $var1 != root ]
then
    echo "Please run the rescript as 'root'."
    exit 1;
fi

PART=0
if [ $# -eq 1 ]
then
    USERNEPTUN=$1;
elif  [ $# -eq 2 ]
then
    PART=$1;
    USERNEPTUN=$2;
else
    echo "Please next time provide the NEPTUN code."
    exit 1;
fi

ccci=0;
cccc=0;
while [ $ccci -lt ${#USERNEPTUN} ];
do
    ccct=$(ord ${USERNEPTUN:$ccci:1})
    let cccc=cccc+ccct
    ccci=$((ccci+1));
done
USERCODE=$((cccc%4))
case $USERCODE in
    0)
       USERCODE=A
       ;;
    1)
       USERCODE=B
       ;;
    2)
       USERCODE=C
       ;;
    3)
       USERCODE=D
       ;;
esac

DATETIME=`date +"%Y. %m. %d., %T"`
echo "Aktuális idő: " $DATETIME
echo "Az Ön NEPTUN kódja:" $USERNEPTUN
echo "Az Ön betűje az ábécéből a:" $USERCODE
echo

# gneral settings
TMPDIR=`mktemp -d`
trap "rm -rf $TMPDIR" EXIT
IP_ADDRESS=$(ip addr show dev ens33 | grep 'inet ' | sed 's/^.*inet \([0-9\.]*\)\/.*$/\1/')
echo "A vituális gép IP címe:" $IP_ADDRESS
PHANTOM_ADDRESS=`echo $IP_ADDRESS | sed 's/\([0-9]\+.[0-9]\+\).*$/\1.37/'`
PHANTOM_ADDRESS="$PHANTOM_ADDRESS.$((RANDOM%100+96))"
HOST_IP=`echo $IP_ADDRESS | sed 's/^\([0-9][0-9]*.[0-9][0-9]*.[0-9][0-9]*\).*$/\1/'`
HOST_IP="$HOST_IP.1"
echo "A gazadgép IP címe:" $HOST_IP

echo 1 > /proc/sys/net/ipv4/ip_forward

if [ "$PART" -eq 1 ] || [ "$PART" -eq 0 ];
then
    echo -n "A mysql újraindítása..................."
    if [ -f /etc/init.d/mysql ]
    then
        mysql_found=1;
        /etc/init.d/mysql restart >/dev/null 2>/dev/null
        echo "....[SIKERÜLT]"
    else
        mysql_found=0;
        echo "[NEM SIKERÜLT]"
    fi
fi

if [ "$PART" -eq 3 ] || [ "$PART" -eq 0 ];
then
    echo -n "Az Apache újraindítása................."
    if [ -f /etc/init.d/apache2 ]
    then
        apache_found=1;
        /etc/init.d/apache2 restart >/dev/null 2>/dev/null
        echo "....[SIKERÜLT]"
    else
        apache_found=0;
        echo "[NEM SIKERÜLT]"
    fi
fi

echo

echo -n "Tsoukalos regisztrálása................"
cut -d: -f1 /etc/passwd | grep tsoukalos | awk '{ system("deluser --remove-home " $1 " > /dev/null 2> /dev/null") }'
tsoukalos_name="tsoukalos_$((RANDOM%1000))"
tsoukalos_password="tsoukybaby"
pass=$(perl -e 'print crypt($ARGV[0],"password")' $tsoukalos_password) #"'
useradd -m -p $pass $tsoukalos_name
echo "....[SIKERÜLT]"

echo -n "Csaló regisztrálása...................."
doublespeak_name="kamunev_ooo_666_nmngdfhgiogheoirhjsfcakjbn__6464sdf"
grep "$doublespeak_name" /etc/passwd >/dev/null
while [ $? -eq 0 ]; do
    doublespeak_name="kamunev_ooo_666_nmngdfhgiogheoirhjsfcakjbn__6464sdf_$((RANDOM%1000))"
    grep "$doublespeak_name" /etc/passwd >/dev/null
done
echo "....[SIKERÜLT]"

echo

############################################################
############################################################


# variables to store points
sub_1=0 sub_2=0 sub_3=0
all_1=0 all_2=0 all_3=0
exercise_ok=0
exercise_failed=0
exercise_optional=0
CODE="($DATETIME, $USERNEPTUN):"; # prerequisites ok


if [ "$PART" -eq 1 ] || [ "$PART" -eq 0 ];
then
    echo
    echo ">>>>> 1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
    echo
    SECTION=1

    ## 1.5 #########################################################x
    chk=0;
    echo "1.5. feladat:"
    echo -n "Pingelés 1............................."
    ping 127.0.0.1 -c 1 -W 2 >/dev/null
    if [ $? -eq 1 ]
    then
        echo "....[SIKERÜLT]"
        ((chk++))
    else
        echo "[NEM SIKERÜLT]"
    fi

    echo -n "Pingelés 2............................."
    ping $IP_ADDRESS -c 1 -W 2 >/dev/null
    if [ $? -eq 1 ] 
    then
        echo "....[SIKERÜLT]"
        ((chk++))
    else
        echo "[NEM SIKERÜLT]"
    fi

    if [ $chk -eq 2 ];
    then
        std_ready
    else
        std_failed
    fi

    ## 1.6 #########################################################x
    echo "1.6. feladat:"
    echo -n "Felhasználók ellenőrzése..............."
    mekkelek_found=`cat /etc/passwd | grep mekkelek -c`

    if [ $mekkelek_found -eq 1 ] 
    then
        echo "....[SIKERÜLT]"
        std_ready
    else
        echo "[NEM SIKERÜLT]"
        std_failed
    fi

    ## 1.4 #########################################################x
    chk=0
    echo "1.4. feladat:"
    echo -n "SSH ellenőrzése/laboruser/helyi........"
    sshpass -p "laboruser" ssh -l laboruser $IP_ADDRESS -o StrictHostKeyChecking=no -o ConnectTimeout=5 -o LogLevel=quiet 'echo -n 2>&1' && var2=1 || var2=0 1>/dev/null 2>/dev/null
    if [ $var2 -eq 1 ] 
    then
        echo "....[SIKERÜLT]"
        ((chk++))
    else
        echo "[NEM SIKERÜLT]"
    fi

    echo -n "SSH ellenőrzése/laboruser/távoli......."
    drp0=$(iptables -L INPUT | grep policy | grep -c ACCEPT)
    if [ $drp0 -eq 1 ];
    then
        drp1=$(iptables -L INPUT -v -n | grep -E 'DROP|REJECT' | tr -s " " | cut -f2 -d" "| awk '{s+=$1} END {print s}')
        sendip $IP_ADDRESS -d "almafa" -p ipv4 -is $PHANTOM_ADDRESS -p tcp -td 22 > /dev/null 2> /dev/null
        drp2=$(iptables -L INPUT -v -n | grep -E 'DROP|REJECT' | tr -s " " | cut -f2 -d" "| awk '{s+=$1} END {print s}')
        if [ -n "$drp1" ] && [ -n "$drp2" ] && [ "$drp2" -gt "$drp1" ];
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi
    else
        drp1=$(iptables -L INPUT -v -n | grep -E 'policy DROP' | tr -s " " | cut -f5 -d" "| awk '{s+=$1} END {print s}')
        sendip $IP_ADDRESS -d "almafa" -p ipv4 -is $PHANTOM_ADDRESS -p tcp -td 22 > /dev/null 2> /dev/null
        drp2=$(iptables -L INPUT -v -n | grep -E 'policy DROP' | tr -s " " | cut -f5 -d" "| awk '{s+=$1} END {print s}')
        if [ -n "$drp1" ] && [ -n "$drp2" ] && [ "$drp2" -gt "$drp1" ];
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi
    fi

    if [ $chk -eq 2 ];
    then
        std_ready
    else
        std_failed
    fi


    ## 1.7 #########################################################x
    echo "1.7. feladat:"
    echo -n "Felhasználói jogosult. ellenőrzése....."
    if [ $mekkelek_found -eq 1 ] && [ -f /etc/sudoers ];
    then

        var2a=$(cat /etc/sudoers | sed 's/[ \t]*//g' | grep "mekkelekALL=(ALL:ALL)ALL" -c)
        var2b=$(groups mekkelek | grep -c "sudo")

        if [ $var2a -eq 1 ] || [ $var2b -eq 1 ]
        then
            echo "....[SIKERÜLT]"
            std_ready
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi
    else
        echo "[NEM SIKERÜLT]"
        std_failed
    fi

    ## 1.8 #########################################################x
    echo "1.8. feladat:"
    echo -n "root kitiltása sshd_configgal.........."
    var2=`cat /etc/ssh/sshd_config | grep "[ ]*PermitRootLogin[ ]*no" -c`
    if [ $var2 -eq 1 ]
    then
        echo "....[SIKERÜLT]"
        std_ready
    else
        echo "[NEM SIKERÜLT]"
        std_failed
    fi

    ## 1.10 ########################################################x
    mysql_root=0;
    if [ $mysql_found -eq 1 ]
    then
        echo "1.10. feladat:"
        echo -n "mysql megnyitása rendszergazdaként....."
        mysql -u root -proot -e '' 1>/dev/null 2>/dev/null
        if [ $? -eq 0 ]
        then
            mysql_root=1   # we can login, do other mysql related exercises

            mysql -u root -palmafa -e '' 1>/dev/null 2>/dev/null
            if [ $? -eq 0 ]
            then
                echo "[NEM SIKERÜLT]"
                std_failed
            else
                echo "....[SIKERÜLT]"
                std_ready
            fi
        fi
    else
        echo "1.10. feladat:"
        echo "mysql megnyitása rendszergazdaként.....[NEM SIKERÜLT]"
        std_failed
    fi

    if [ $mysql_root -eq 1 ];
    then

        ## 1.14 ########################################################x
        echo "1.14. feladat:"
        echo -n "students adatbázis keresése............"
        var2=`mysql -u root -proot -e 'SHOW DATABASES;' | grep "students" -c`
        if [ $var2 -eq 1 ] 
        then
            echo "....[SIKERÜLT]"
            std_ready
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi

        ## 1.16 ########################################################x
        if [ $var2 -eq 1 ];
        then
            chk=0
            echo "1.16. feladat:"
            echo -n "Mekk Elek az új tanuló.............."
            var2=`mysql -u root -proot -e 'SELECT * FROM students.students WHERE name="Mekk Elek";' | grep "Mekk Elek" -c`
            var3=`mysql -u root -proot -e 'SELECT * FROM students.students WHERE name="Mekk Elek";' | grep "$USERNEPTUN" -c`
            if [ $var3 -eq 1 ] 
            then
                echo "[FELVÉTELT NYERT]"
                ((chk++))
            else
                if [ $var2 -eq 1 ]
                then
                    echo "[FELVÉTELT NYERT] (rossz NEPTUN kóddal)"
                    ((chk++))
                else
                    echo ".......[ELBUKOTT]"
                fi
            fi

            echo -n "Táblák sorainak számlálása............."
            var2=`mysql -u root -proot -e 'SELECT COUNT(*) FROM students.courses;' | grep "[0-9 ][0-9]*"`
            if [ $var2 -gt 9 ]
            then
                var3=`mysql -u root -proot -e 'SELECT COUNT(*) FROM students.results;' | grep "[0-9 ][0-9]*"`
                if [ $var3 -gt 25 ]
                then
                    echo "....[SIKERÜLT]"
                    ((chk++))
                else
                    echo "[NEM SIKERÜLT]"
                fi
            else
                echo "[NEM SIKERÜLT]"
            fi

            if [ $chk -eq 2 ]
            then
                opt_ready
            else
                opt_failed
            fi

        else
            ## 1.16 ########################################################x
            echo "1.16. feladat:"
            echo "Mekk Elek az új tanuló.....................[ELBUKOTT]"
            echo "Táblák sorainak számlálása.............[NEM SIKERÜLT]"
            opt_failed
        fi

    else

        ## 1.14 ########################################################x
        echo "1.14. feladat:"
        echo "students adatbázis keresése............[NEM SIKERÜLT]"
        std_failed

        ## 1.16 ########################################################x
        echo "1.16. feladat:"
        echo "Mekk Elek az új tanuló.....................[ELBUKOTT]"
        echo "Táblák sorainak számlálása.............[NEM SIKERÜLT]"
        opt_failed
    fi

fi

if [ "$PART" -eq 2 ] || [ "$PART" -eq 0 ];
then

    echo
    echo ">>>>> 2 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
    echo
    SECTION=2

    # apache is installed!!!
    ## 2.1
    echo "2.1. feladat:"
    echo -n "Böngészés ellenőrzése.................."
    wget $IP_ADDRESS -t 1 -q --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -O /dev/null
    if [ $? -eq 0 ]
    then
        echo "....[SIKERÜLT]"
        std_ready
    else
        ctest=$(apache2ctl -t 2>&1 >/dev/null | grep -c "Syntax OK")
        if [ $ctest -eq 1 ];
        then
            echo "....[SIKERÜLT]"
            std_ready
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi
    fi

    found=`apache2ctl -S 2>/dev/null | grep -c "irulabor.vmware"`
    if [ $found -ne 0 ];
    then
        ## 2.3
        echo "2.3. feladat:"
        echo -n "Böngészés: irulabor.vmware............."
        wget irulabor.vmware -t 1 -q --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -O /dev/null
        if [ $? -eq 0 ];
        then
            echo "....[SIKERÜLT]"
            std_ready
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi

        ## 2.4
        chk=0
        echo "2.4. feladat:"

        echo -n "Böngészés: irulabor.vmware/vedett......"

        var1=$(curl --interface $IP_ADDRESS -H 'Cache-Control: no-cache' -I -u mekkelek:$USERNEPTUN irulabor.vmware/vedett/index.html 2>/dev/null | grep -c "403 Forbidden")
        if [ $var1 -eq 1 ]
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        if [ $chk -eq 1 ]
        then
            std_ready
        else
            std_failed
        fi

        ## 2.5
        chk=0
        echo "2.5. feladat:"
        echo -n "Böngészés: irulabor.vmware/vedett l1..."
        wget --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/vedett/
        if [ $? -ne 0 ];
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        echo -n "Böngészés: irulabor.vmware/vedett l2..."
        wget --user=mekkelek --password=$USERNEPTUN --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/vedett/
        if [ $? -eq 0 ];
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        echo -n "Böngészés: irulabor.vmware/vedett l3..."
        wget --user=mekkelek --password=ALMAFA5 --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/vedett/
        if [ $? -ne 0 ]
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        echo -n "Böngészés: irulabor.vmware/vedett l4..."
        wget --user="$doublespeak_name" --password=ALMAFA5___0 --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/vedett/
        if [ $? -ne 0 ]
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        if [ $chk -eq 4 ]
        then
            std_ready
        else
            std_failed
        fi

        ## 2.6
        chk=0
        echo "2.6. feladat:"
        echo -n "Böngészés: nagyonvedett l1............."
        wget --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/nagyonvedett/
        if [ $? -ne 0 ];
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        echo -n "Böngészés: nagyonvedett l2............."
        wget --user="$tsoukalos_name" --password="$tsoukalos_password" --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/nagyonvedett/
        if [ $? -eq 0 ]
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        echo -n "Böngészés: nagyonvedett l3............."
        wget --user="$tsoukalos_name" --password=ALMAFA5 --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/vedett/
        if [ $? -ne 0 ]
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        echo -n "Böngészés: nagyonvedett l4............."
        wget --user="$doublespeak_name" --password=ALMAFA5___0 --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/vedett/
        if [ $? -ne 0 ]
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi

        if [ $chk -eq 4 ]
        then
            std_ready
        else
            std_failed
        fi

        ## 2.8
        echo "2.8. feladat:"
        echo -n "Böngészés: nyilvános, .htaccess........"
        aoen=0 # htaccess not enabled

        vhost=$(apache2ctl -S 2>/dev/null | grep "irulabor.vmware" | sed 's/[^(]*(\([^:]*\):.*/\1/')
        found2=$(cat $vhost | grep -c "nyilvanos")
        if [ $found2 -ne 0 ];
        then
            hsname="/"$(cat $vhost | grep "DocumentRoot" | cut -d'/' -f2-)
            if [ "${hsname: -1}" = "/" ]; then
                hsname="${hsname}nyilvanos/.htaccess"
            else
                hsname="$hsname/nyilvanos/.htaccess"
            fi
            if [ -f "$hsname" ];
            then
                hres1=$(wget --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/nyilvanos/ 2>&1 | grep -c "ERROR")
                sed -i '1iERROR' "$hsname"
                hres2=$(wget --server-response --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O /dev/null http://irulabor.vmware/nyilvanos/ 2>&1 | grep -c "HTTP/1.1 500")
                if [ $hres1 -eq 0 ] && [ $hres2 -gt 0 ]; then
                    echo "....[SIKERÜLT]"
                    std_ready
                    aoen=1
                else
                    echo "[NEM SIKERÜLT]"
                    std_failed
                fi
                sed -i '1d' "$hsname"
            else
                echo "[NEM SIKERÜLT]"
                std_failed
            fi
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi

        ## 2.9
        echo "2.9. feladat:"
        echo -n "Böngészés: nyilvános, listázás........."
        if [ $found2 -ne 0 ] && [ $aoen -ne 0 ];
        then
            tmp_web1=`mktemp -p $TMPDIR`
            tmp_web2=`mktemp -p $TMPDIR`
            wget --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O $tmp_web1 http://irulabor.vmware/nyilvanos >/dev/null 2>/dev/null
            if [ $? -eq 0 ];
            then
                wget --no-dns-cache --no-http-keep-alive --no-cache --no-cookies -t 1 -q -O $tmp_web2 http://irulabor.vmware/nyilvanos/nyilvanos.html >/dev/null 2>/dev/null
                if [ $? -eq 0 ];
                then
                    diff -B -b -E -q -s $tmp_web1 $tmp_web2 >/dev/null 2>/dev/null
                    if [ $? -ne 0 ];
                    then
                        echo "....[SIKERÜLT]"
                        std_ready
                    else
                        echo "[NEM SIKERÜLT]"
                        std_failed
                    fi
                else
                    echo "[NEM SIKERÜLT]"
                    std_failed
                fi
            else
                echo "[NEM SIKERÜLT]"
                std_failed
            fi
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi
    else
        CODE="$CODE 0 0 0 0 0 0"
        echo "2.3. feladat:"
        echo "Böngészés: irulabor.vmware.............[NEM SIKERÜLT]"
        std_failed
        echo "2.4. feladat:"
        echo "Böngészés: irulabor.vmware/vedett r....[NEM SIKERÜLT]"
        std_failed
        echo "2.5. feladat:"
        echo "Böngészés: irulabor.vmware/vedett l....[NEM SIKERÜLT]"
        std_failed
        echo "2.6. feladat:"
        echo "Böngészés: nagyonvedett................[NEM SIKERÜLT]"
        std_failed
        echo "2.8. feladat:"
        echo "Böngészés: nyilvános, .htaccess........[NEM SIKERÜLT]"
        std_failed
        echo "2.9. feladat:"
        echo "Böngészés: nyilvános, listázás.........[NEM SIKERÜLT]"
        std_failed
    fi

fi


if [ "$PART" -eq 3 ] || [ "$PART" -eq 0 ];
then

echo
echo ">>>>> 3 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
echo

SECTION=3

## 5.1
echo "3.1. feladat:"
case $USERCODE in
    "A")
         echo -n "Vendor id.............................."
         if [ -f /home/laboruser/bin/3_1.sh ]
         then
             chmod 777 /home/laboruser/bin/3_1.sh
             diff -B -b -i -E -q -s <(cat /proc/cpuinfo | grep "vendor_id" | sed -e 's/.*://g' | sed -e 's/^[ \t]*//' | tr -d ' ') <(/home/laboruser/bin/3_1.sh | sed -e 's/^[ \t]*//' | tr -d ' ') >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "B")
         echo -n "Vendor id.............................."
         if [ -f /home/laboruser/bin/3_1.sh ]
         then
             chmod 777 /home/laboruser/bin/3_1.sh
             diff -B -b -i -E -q -s <(cat /proc/cpuinfo | grep "vendor_id" | sed -e 's/.*://g' | sed -e 's/^[ \t]*//' | tr -d ' ') <(/home/laboruser/bin/3_1.sh | sed -e 's/^[ \t]*//' | tr -d ' ') >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "C")
         echo -n "Processzor frekvenciája................"
         if [ -f /home/laboruser/bin/3_1.sh ]
         then
             chmod 777 /home/laboruser/bin/3_1.sh
             diff -B -b -i -E -q -s <(cat /proc/cpuinfo | grep "cpu MHz" | sed -e 's/.*://g' | sed -e 's/^[ \t]*//' | tr -d ' ') <(/home/laboruser/bin/3_1.sh | sort | sed -e 's/^[ \t]*//' | tr -d ' ') >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "D")
         echo -n "Processzor frekvenciája................"
         if [ -f /home/laboruser/bin/3_1.sh ]
         then
             chmod 777 /home/laboruser/bin/3_1.sh
             diff -B -b -i -E -q -s <(cat /proc/cpuinfo | grep "cpu MHz" | sed -e 's/.*://g' | sed -e 's/^[ \t]*//' | tr -d ' ') <(/home/laboruser/bin/3_1.sh | sort | sed -e 's/^[ \t]*//' | tr -d ' ') >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
esac

if [ $? -eq 0 ]
then
    echo "....[SIKERÜLT]"
    std_ready
else
    echo "[NEM SIKERÜLT]"
    std_failed
fi

## 3.2
echo "3.2. feladat:"
echo -n "Üres sorok (1)........................."

if [ -f /home/laboruser/bin/3_2.sh ]
then
    chmod 777 /home/laboruser/bin/3_2.sh
    diff -B -b -i -E -q -s <(echo -e "as as as\n\n  as \nnasas\n \t123\n" | grep -c "^$") <(echo -e "as as as\n\n  as \nnasas\n \t123\n" | /home/laboruser/bin/3_2.sh) >/dev/null 2>/dev/null
    if [ $? -eq 0 ]
    then
        echo "....[SIKERÜLT]"
        echo -n "Üres sorok (2)........................."

        diff -B -b -i -E -q -s <(echo -e " \n  \n   \n\t\n123\n \t\n " | grep -c "^$") <(echo -e " \n  \n   \n\t\n123\n \t\n " | /home/laboruser/bin/3_2.sh) >/dev/null 2>/dev/null
        if [ $? -eq 0 ]
        then
            echo "....[SIKERÜLT]"
            std_ready
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi
    else
        echo "[NEM SIKERÜLT]"
        echo -n "Üres sorok (2)........................."
        diff -B -b -i -E -q -s <(echo -e " \n  \n   \n\t\n123\n " | grep -c "^$") <(echo -e " \n  \n   \n\t\n123\n " | /home/laboruser/bin/3_2.sh) >/dev/null 2>/dev/null
        if [ $? -eq 0 ]
        then
            echo "....[SIKERÜLT]"
            std_failed
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi
    fi
else
    echo "[NEM SIKERÜLT]"
    echo "Üres sorok (2).........................[NEM SIKERÜLT]"
    std_failed
fi

## 3.3
echo "3.3. feladat:"
echo -n "Maradék................................"

case $USERCODE in
    "A")
         if [ -f /home/laboruser/bin/3_3.sh ]
         then
             chmod 777 /home/laboruser/bin/3_3.sh
             diff -B -b -i -E -q -s -w <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%7==0) print $1 " " $5 " " $3 " " $4 " " $2}' ) <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 7) >/dev/null 2>/dev/null
             if [ $? -eq 0 ]
             then
                 diff -B -b -i -E -q -s -w <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%5==0) print $1 " " $5 " " $3 " " $4 " " $2}' ) <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 5) >/dev/null 2>/dev/null
             else
                 diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
             fi
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "C")
         if [ -f /home/laboruser/bin/3_3.sh ]
         then
             chmod 777 /home/laboruser/bin/3_3.sh
             diff -B -b -i -E -q -s -w <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%7==0) print $1 " " $5 " " $3 " " $4 " " $2}' ) <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 7) >/dev/null 2>/dev/null
             if [ $? -eq 0 ]
             then
                 diff -B -b -i -E -q -s -w <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%5==0) print $1 " " $5 " " $3 " " $4 " " $2}' ) <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 5) >/dev/null 2>/dev/null
             else
                 diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
             fi
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "B")
         if [ -f /home/laboruser/bin/3_3.sh ]
         then
             chmod 777 /home/laboruser/bin/3_3.sh
             diff -B -b -i -E -q -s -w <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%7==0) print $1 " " $2 " " $3 " " $4 $5}' ) <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 7) >/dev/null 2>/dev/null
             if [ $? -eq 0 ]
             then
                 diff -B -b -i -E -q -s -w <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%5==0) print $1 " " $2 " " $3 " " $4 $5}' ) <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 5) >/dev/null 2>/dev/null
             else
                 diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
             fi
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "D")
         if [ -f /home/laboruser/bin/3_3.sh ]
         then
             chmod 777 /home/laboruser/bin/3_3.sh
             diff -B -b -i -E -q -s -w <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%7==0) print $1 " " $2 " " $3 " " $4 $5}' ) <(echo -e '3 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 7) >/dev/null 2>/dev/null
             if [ $? -eq 0 ]
             then
                 diff -B -b -i -E -q -s -w <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | awk -F" " '/^[1-9][0-9]* /{if($1%5==0) print $1 " " $2 " " $3 " " $4 $5}' ) <(echo -e '100 a3 b3 c3 d3\n777 a7 b7 c7 Berci\n15 i j k l\n1a i j k l\na2 i1 j1 k1 l1\n077 i2 j2 k2 l2\n14 almafa kortefa kucko barack\n73 a73 b73 c73 d73' | /home/laboruser/bin/3_3.sh 5) >/dev/null 2>/dev/null
             else
                 diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
             fi
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
esac

if [ $? -eq 0 ]
then
    echo "....[SIKERÜLT]"
    std_ready
else
    echo "[NEM SIKERÜLT]"
    std_failed
fi


## 3.4
echo "3.4. feladat:"

echo -n "Klónozás..............................."
iptables -t filter -I INPUT 1 -p tcp --dport ssh -j ACCEPT
screen -dmS "backdoor1$USERNEPTUN" sshpass -p laboruser ssh -l laboruser "$IP_ADDRESS"
screen -dmS "backdoor2$USERNEPTUN" sshpass -p laboruser ssh -l laboruser "$IP_ADDRESS"
sleep 1
screen -dmS "backdoor3$USERNEPTUN" sshpass -p tsoukybaby ssh -l "$tsoukalos_name" "$IP_ADDRESS"
sleep 2
screen -dmS "backdoor4$USERNEPTUN" sshpass -p laboruser ssh -l laboruser "$IP_ADDRESS"
sleep 1

echo "....[SIKERÜLT]"
echo -n "Ki vagyok én?.........................."
if [ -f /home/laboruser/bin/3_4.sh ];
then
    chmod 777 /home/laboruser/bin/3_4.sh

    var2a=$(runuser -l laboruser /home/laboruser/bin/3_4.sh | grep -c "laboruser")
    var2b=$(/home/laboruser/bin/3_4.sh | grep -c "root")

    var3t=$(who -b | cut -dt -f3 | cut -d' ' -f3-)
    var3a=$(cat /home/laboruser/bin/3_4.sh | grep -c "$var3t")
    var3b=$(/home/laboruser/bin/3_4.sh | grep -c "$var3t")

    var4t=$(uptime -s | cut -d: -f-2)
    var4a=$(cat /home/laboruser/bin/3_4.sh | grep -c "$var4t")
    var4b=$(/home/laboruser/bin/3_4.sh | grep -c "$var4t")

    var5a=$(( $var3a + $var4a ))
    var5b=$(( $var3b + $var4b ))

    cp /home/laboruser/bin/3_4.sh /home/laboruser/bin/3_4.bak.sh
    echo -e "\necho \"\$\$\"" >> /home/laboruser/bin/3_4.sh
    var2c=`/home/laboruser/bin/3_4.sh | tail -n 2 | uniq | wc -l`

    mv /home/laboruser/bin/3_4.bak.sh /home/laboruser/bin/3_4.sh

    if [ $var2a -ge 1 ] && [ $var2b -ge 1 ] && [ $var2c -eq 1 ] && [ $var5a -eq 0 ] && [ $var5b -ge 1 ];
    then
        diff -B -b -i -E -q -s <(who -u | tr -s ' ' | cut -f1 -d' ' | sort | uniq) <(/home/laboruser/bin/3_4.sh | sed '1,2d' | head -n -2 | sort) >/dev/null 2>/dev/null
        if [ $? -eq 0 ];     # logged in users
        then
            var2a=`date +'%Y. %m. %d.'`
            var2b=`/home/laboruser/bin/3_4.sh | grep "$var2a" -c`
            var2c=`date +'%Y. %m. %d.' | grep "$var2a" -c`
            if [ $var2b -ge 1 ];
            then
                echo "....[SIKERÜLT]"
                std_ready
            else
                if [ $var2c -ne 1 ];
                then
                    var2a=`date +'%Y. %m. %d.'`
                    var2b=`/home/laboruser/bin/3_4.sh | grep "$var2a" -c`
                    if [ $var2b -ge 1 ];
                    then
                        echo "....[SIKERÜLT]"
                        std_ready
                    else
                        echo "[NEM SIKERÜL1]"
                        std_failed
                    fi
                else
                    echo "[NEM SIKERÜL2]"
                    std_failed
                fi
            fi
        else
            echo "[NEM SIKERÜL3]"
            std_failed
        fi
    else
        echo "[NEM SIKERÜL4]"
        std_failed
    fi
else
    echo "[NEM SIKERÜLT]"
    std_failed
fi

echo -n "Klónok (ex)terminálása................."
iptables -t filter -D INPUT 1
screen -S "backdoor1$USERNEPTUN" -p 0 -X quit 2> /dev/null 1> /dev/null
screen -S "backdoor2$USERNEPTUN" -p 0 -X quit 2> /dev/null 1> /dev/null
screen -S "backdoor3$USERNEPTUN" -p 0 -X quit 2> /dev/null 1> /dev/null
screen -S "backdoor4$USERNEPTUN" -p 0 -X quit 2> /dev/null 1> /dev/null
echo "....[SIKERÜLT]"
echo

## 3.5 (opcionális)
echo "3.5. feladat:"
chk=0
if [ -f /home/laboruser/bin/3_5.sh ];
then
    chmod 777 /home/laboruser/bin/3_5.sh

    echo -n "CSV konvertálás (,)...................."
    res=`echo "1,\"5,5\",6" | /home/laboruser/bin/3_5.sh`
    cnt1=`echo $res | grep -o ";" | wc -l`
    if [ $cnt1 -eq 6 ];
    then
        cnt1=`echo $res | grep -c "5,5"`
        if [ $cnt1 -eq 1 ];
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi
    else
       echo "[NEM SIKERÜLT]"
    fi

    echo -n "CSV konvertálás (;)...................."
    res=`echo "\"alma,szilva\",34;56,0" | /home/laboruser/bin/3_5.sh`
    cnt1=`echo $res | grep -c "^\"alma,szilva\";\"34;56\";"`
    cnt2=`echo $res | grep -c "^alma,szilva;\"34;56\";"`
    if [ $cnt1 -eq 1 ] || [ $cnt2 -eq 1 ];
    then
        echo "....[SIKERÜLT]"
        ((chk++))
    else
       echo "[NEM SIKERÜLT]"
    fi

    echo -n "CSV konvertálás (,;)..................."
    res=`echo "1,\",a,;bd,\",\"3125\"" | /home/laboruser/bin/3_5.sh`
    cnt1=`echo $res | grep -o ";" | wc -l`
    if [ $cnt1 -eq 3 ];
    then
        cnt1=`echo $res | grep -c "\",a,;bd,\""`
        if [ $cnt1 -eq 1 ]
        then
            echo "....[SIKERÜLT]"
            ((chk++))
        else
            echo "[NEM SIKERÜLT]"
        fi
    else
       echo "[NEM SIKERÜLT]"
    fi

    if [ $chk -eq 3 ];
    then
        opt_ready
    else
        opt_failed
    fi

else
    echo "CSV konvertálás (,)....................[NEM SIKERÜLT]"
    echo "CSV konvertálás (;)....................[NEM SIKERÜLT]"
    echo "CSV konvertálás (,;)...................[NEM SIKERÜLT]"
    opt_failed
fi


## 3.6
echo "3.6. feladat:"
echo -n "diff..................................."

if [ -f /home/laboruser/bin/3_6.sh ];
then
    chmod 777 /home/laboruser/bin/3_6.sh

    echo -e 'a\nb\nc\nd' > .file.1
    echo -e 'a\nb\nc\nD' > .file.2

    diff -B -b -i -E -q -s -w <(echo -e '1\n1' ) <(/home/laboruser/bin/3_6.sh .file.1 .file.2) >/dev/null 2>/dev/null

    if [ $? -eq 0 ]
    then

        echo -e 'b\naAa\n' >> .file.1
        echo -e 'b\naBa\nx\n' >> .file.2

        diff -B -b -i -E -q -s -w <(echo -e '2\n3' ) <(/home/laboruser/bin/3_6.sh .file.1 .file.2) >/dev/null 2>/dev/null

        if [ $? -eq 0 ]
        then
            echo "....[SIKERÜLT]"
            std_ready
        else
            echo "[NEM SIKERÜLT]"
            std_failed
        fi
    else
        echo "[NEM SIKERÜLT]"
        std_failed
    fi

    rm .file.1
    rm .file.2
else
    echo "[NEM SIKERÜLT]"
    std_failed
fi


## 3.7
echo "3.7. feladat:"
echo -n "Csoportok.............................."

case $USERCODE in
    "A")
         if [ -f /home/laboruser/bin/3_7.sh ]
         then
             chmod 777 /home/laboruser/bin/3_7.sh
             diff -B -b -i -E -q -s -w <(echo -e 'i1: 150\ni2: 1000\ni3: 5000' ) <(echo -e 'i1,1,100\ni2,2,200\ni1,1,50\ni2,2,300\ni3,5,1000' | /home/laboruser/bin/3_7.sh | sort) >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "C")
         if [ -f /home/laboruser/bin/3_7.sh ]
         then
             chmod 777 /home/laboruser/bin/3_7.sh
             diff -B -b -i -E -q -s -w <(echo -e 'i1: 1,100\ni2: 2,200\ni3: 5,1000' ) <(echo -e 'i1,1,100\ni2,2,200\ni1,1,50\ni2,2,300\ni3,5,1000' | /home/laboruser/bin/3_7.sh | sort) >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "B")
         if [ -f /home/laboruser/bin/3_7.sh ]
         then
             chmod 777 /home/laboruser/bin/3_7.sh
             diff -B -b -i -E -q -s -w <(echo -e 'i1: 55\ni2: 200\ni3: 1000\ni7: 11' ) <(echo -e 'i7,12,11\ni1,1,100\ni2,2,200\ni1,1,55\ni2,2,300\ni3,5,1000' | /home/laboruser/bin/3_7.sh | sort) >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
    "D")
         if [ -f /home/laboruser/bin/3_7.sh ]
         then
             chmod 777 /home/laboruser/bin/3_7.sh
             diff -B -b -i -E -q -s -w <(echo -e 'i1: 2\ni2: 3\ni3: 1\i5: 2' ) <(echo -e 'i5,1,1\ni1,1,100\ni2,2,200\ni1,1,50\ni2,2,300\ni3,5,1000\ni2,7,7\ni5,1,1' | /home/laboruser/bin/3_7.sh | sort) >/dev/null 2>/dev/null
         else
             diff -q  <(less /proc/cpuinfo | grep "model name") <(less /proc/cpuinfo | grep "vendor id") >/dev/null 2>/dev/null
         fi
         ;;
esac

if [ $? -eq 0 ]
then
    echo "....[SIKERÜLT]"
    std_ready
else
    echo "[NEM SIKERÜLT]"
    std_failed
fi

## 3.8 (opcionális)
echo "3.8. feladat:"
echo -n "Gyakoriság............................."

if [ -f /home/laboruser/bin/3_8.sh ];
then
    chmod 777 /home/laboruser/bin/3_8.sh
    diff -B -b -i -E -q -s -w <(echo '1,0,0,0,4,1,1,0,0,1' ) <(echo -e '1;0.1;2\n2;0.62;15\n1;0.5;35\n15;0.97;45\n1;0.46;35\n1;0.47;35\n1;0.41;35\n1;0.51;37\n' | /home/laboruser/bin/3_8.sh) >/dev/null 2>/dev/null
    echo "....[SIKERÜLT]"
    opt_ready
else
    echo "[NEM SIKERÜLT]"
    opt_failed
fi

fi


echo -n "Tsoukalos törlése......................"
cut -d: -f1 /etc/passwd | grep tsoukalos | awk '{ system("deluser --remove-home " $1 " > /dev/null 2> /dev/null") }'
echo "....[SIKERÜLT]"


######################################################
######################################################
######################################################

## verdikt
if [ "$all_1" -eq 0 ];
then
    part_1=0
else
    part_1=`echo "scale=2;100*$sub_1/$all_1" | bc -l`
fi
if [ "$all_2" -eq 0 ];
then
    part_2=0
else
    part_2=`echo "scale=2;100*$sub_2/$all_2" | bc -l`
fi
if [ "$all_3" -eq 0 ];
then
    part_3=0
else
    part_3=`echo "scale=2;100*$sub_3/$all_3" | bc -l`
fi
defficit=`echo "define m(x,y){ if(x>y) return (x) else return (y) } (m($all_1*(40-$part_1),0)/100)+(m($all_2*(40-$part_2),0)/100)+(m($all_3*(40-$part_3),0)/100)" | bc -l`

echo
echo
echo "SIKERES RÉSZFELADAT:                  " $exercise_ok
echo "SIKERES (opcionális) RÉSZFELADAT:     " $exercise_optional
echo "ELRONTOTT RÉSZFELADAT:                " $exercise_failed
echo "DEFICIT:                              " $defficit
echo
echo "%:                  " $part_1 $part_2 $part_3
echo
echo -n "ÍTÉLET:     "
verdict=0
if [ $( echo "$part_1>=40" | bc ) -eq 1 ] && [ $( echo "$part_2>=40" | bc ) -eq 1 ] && [ $( echo "$part_3>=40" | bc ) -eq 1 ];
then
    verdict=1
    echo "SIKERÜLT"
else
    if [ $( echo "$defficit<=$exercise_optional" | bc ) -eq 1 ] && [ "$part_1" != "0" ] &&  [ "$part_2" != "0" ] && [ "$part_3" != "0" ];
    then
        verdict=1
        echo "SIKERÜLT"
    else
        echo "NEM SIKERÜLT"
    fi
fi
echo "Ellenőrző összeg:"
echo $CODE "::" $part_1 $part_2 $part_3 "::" $verdict | fold -w 40
echo
echo "Kód: "
# Add some secrete!
CHECKSUM=`echo "iru_nemeth_$CODE xXx $defficit / / /666__v:$verdict: $part_1, $part_2, $part_3" | sha1sum | sed 's/  -// '`
echo $CHECKSUM | fold -w 40

echo
echo
echo "         -- jegyzőkönyv vége --"
echo "         -- Linux admin labor --"
echo "   Információs rendszerek üzemeltetése"
echo
echo

exit 0;
