add_rules("mode.debug", "mode.release", "mode.releasedbg")

if is_plat("windows") then
    add_cxflags("/utf-8", {force = true}) -- MSVC
    add_defines("UNICODE", "_UNICODE")
    -- 启用 LTCG 链接时代码生成（用于链接 /GL 编译的库）
    if is_mode("release", "releasedbg") then
        add_ldflags("/LTCG")
        -- 可选：同时启用编译时 /GL（如果你自己代码也想优化）
        -- add_cxflags("/GL")
    end
else
    add_cxflags("-finput-charset=UTF-8", "-fexec-charset=UTF-8", {force = true}) -- GCC/Clang
    add_cxflags("-std=c++11")
end

option("devlibs")
    set_default(true)
	add_includedirs("$(projectdir)/../../__devLibs_trdlp/inc")
    if is_plat("windows") then
        add_linkdirs("$(projectdir)/../../__devLibs_trdlp/libs")
    elseif is_arch("arm64-v8a") then
        add_linkdirs("$(projectdir)/../../__devLibs_trdlp/libs_4linux/aarch64/libs_x64")
        add_linkdirs("$(projectdir)/../../__devLibs_trdlp/libs_4linux/aarch64/lib3rds_x64")
    else
        add_linkdirs("$(projectdir)/../../__devLibs_trdlp/libs_4linux/x86_64/libs_x64")
        add_linkdirs("$(projectdir)/../../__devLibs_trdlp/libs_4linux/x86_64/lib3rds_x64")
    end
option_end()

target("Xjson")
    set_kind("binary")
    add_options("devlibs")
    add_files("*.cpp")