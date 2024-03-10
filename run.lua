function main(arg)

    local model_name = "./models/"
    if arg == nil then
        model_name = model_name .. "fx991cnx"
    else
        model_name = model_name .. arg
    end
    print("Load Model: ",model_name)
    os.exec("xmake run CasioEmuX "..model_name)
end
