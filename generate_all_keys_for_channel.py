
channel_postfix = "2353" #channel 1

with open("all_page_addresses.txt") as fdpages:
    for page in fdpages:
        tup = page.split(' = ')
        pagesymbols = tup[1].split(',')[0].strip()
        pageaddr = tup[0]

        with open("all_byte_offsets.txt") as fdbytes:
            for byte in fdbytes:                
                bytetup = byte.split(' = ')
                bytesym = bytetup[1].strip()

                print "6", pagesymbols, bytesym, channel_postfix, 0
       
