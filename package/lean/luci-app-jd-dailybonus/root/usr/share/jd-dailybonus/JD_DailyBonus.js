/*************************

京东多合一签到脚本

更新时间: 2020.12.25 17:50 v1.89
有效接口: 40+
脚本兼容: QuantumultX, Surge, Loon, JSBox, Node.js
电报频道: @NobyDa 
问题反馈: @NobyDa_bot 
如果转载: 请注明出处

*************************
【 JSbox, Node.js 说明 】 :
*************************

开启抓包app后, Safari浏览器登录 https://bean.m.jd.com 点击签到并且出现签到日历后, 返回抓包app搜索关键字 functionId=signBean 复制请求头Cookie填入以下Key处的单引号内即可 */

var Key = ''; //单引号内自行填写您抓取的Cookie

var DualKey = ''; //如需双账号签到,此处单引号内填写抓取的"账号2"Cookie, 否则请勿填写

/* 注1: 以上选项仅针对于JsBox或Node.js, 如果使用QX,Surge,Loon, 请使用脚本获取Cookie.
   注2: 双账号用户抓取"账号1"Cookie后, 请勿点击退出账号(可能会导致Cookie失效), 需清除浏览器资料或更换浏览器登录"账号2"抓取.
   注3: 如果复制的Cookie开头为"Cookie: "请把它删除后填入.
   注4: 如果使用QX,Surge,Loon并获取Cookie后, 再重复填写以上选项, 则签到优先读取以上Cookie.
   注5: 如果使用Node.js, 需自行安装'request'模块. 例: npm install request -g
   注6: Node.js或JSbox环境下已配置数据持久化, 填写Cookie运行一次后, 后续更新脚本无需再次填写, 待Cookie失效后重新抓取填写即可.

*************************
【 QX, Surge, Loon 说明 】 :
*************************

初次使用时, app配置文件添加脚本配置,并启用Mitm后, Safari浏览器打开登录 https://bean.m.jd.com ,点击签到并且出现签到日历后, 如果通知获得cookie成功, 则可以使用此签到脚本。 注: 请勿在京东APP内获取!!!

由于cookie的有效性(经测试网页Cookie有效周期最长31天)，如果脚本后续弹出cookie无效的通知，则需要重复上述步骤。 
签到脚本将在每天的凌晨0:05执行, 您可以修改执行时间。 因部分接口京豆限量领取, 建议调整为凌晨签到。

BoxJs订阅地址: https://raw.githubusercontent.com/NobyDa/Script/master/NobyDa_BoxJs.json

*************************
【 配置双京东账号签到说明 】 : 
*************************

正确配置QX、Surge、Loon后, 并使用此脚本获取"账号1"Cookie成功后, 请勿点击退出账号(可能会导致Cookie失效), 需清除浏览器资料或更换浏览器登录"账号2"获取即可.

注: 获取"账号1"或"账号2"的Cookie后, 后续仅可更新该"账号1"或"账号2"的Cookie.
如需写入其他账号,您可开启脚本内"DeleteCookie"选项以清除Cookie
*************************
【Surge 4.2+ 脚本配置】:
*************************

[Script]
京东多合一签到 = type=cron,cronexp=5 0 * * *,wake-system=1,timeout=60,script-path=https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js

获取京东Cookie = type=http-request,pattern=https:\/\/api\.m\.jd\.com\/client\.action.*functionId=signBean,script-path=https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js

[MITM]
hostname = api.m.jd.com

*************************
【Loon 2.1+ 脚本配置】:
*************************

[Script]
cron "5 0 * * *" tag=京东多合一签到, script-path=https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js

http-request https:\/\/api\.m\.jd\.com\/client\.action.*functionId=signBean tag=获取京东Cookie, script-path=https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js

[MITM]
hostname = api.m.jd.com

*************************
【 QX 1.0.10+ 脚本配置 】 :
*************************

[task_local]
# 京东多合一签到
# 注意此为远程路径, 低版本用户请自行调整为本地路径.
5 0 * * * https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js, tag=京东多合一签到, img-url=https://raw.githubusercontent.com/Orz-3/task/master/jd.png,enabled=true

[rewrite_local]
# 获取京东Cookie. 
# 注意此为远程路径, 低版本用户请自行调整为本地路径.
https:\/\/api\.m\.jd\.com\/client\.action.*functionId=signBean url script-request-header https://raw.githubusercontent.com/NobyDa/Script/master/JD-DailyBonus/JD_DailyBonus.js

[mitm]
hostname = api.m.jd.com

*************************/

var LogDetails = false; //是否开启响应日志, true则开启

var stop = 0; //自定义延迟签到,单位毫秒. 默认分批并发无延迟. 延迟作用于每个签到接口, 如填入延迟则切换顺序签到(耗时较长), VPN重启或越狱用户建议填写1, Surge用户请注意在SurgeUI界面调整脚本超时

var DeleteCookie = false; //是否清除Cookie, true则开启.

var boxdis = true; //是否开启自动禁用, false则关闭. 脚本运行崩溃时(如VPN断连), 下次运行时将自动禁用相关崩溃接口(仅部分接口启用), 崩溃时可能会误禁用正常接口. (该选项仅适用于QX,Surge,Loon)

var ReDis = false; //是否移除所有禁用列表, true则开启. 适用于触发自动禁用后, 需要再次启用接口的情况. (该选项仅适用于QX,Surge,Loon)

var out = 0; //接口超时退出, 用于可能发生的网络不稳定, 0则关闭. 如QX日志出现大量"JS Context timeout"后脚本中断时, 建议填写6000

var $nobyda = nobyda();

async function all() {
  merge = {};
  switch (stop) {
    case 0:
      await Promise.all([
        JingDongBean(stop), //京东京豆
        JDTakeaLook(stop), //京东发现-看一看
        JingDongStore(stop), //京东超市
        JingDongWebcasts(stop), //京东直播
        JingRongBean(stop), //金融简单赚钱
        JingRongSteel(stop), //金融钢镚
        JingDongTurn(stop), //京东转盘
        JDFlashSale(stop), //京东闪购
        JDOverseas(stop), //京东国际
        JingDongCash(stop), //京东现金红包
        JDMagicCube(stop, 2), //京东小魔方
        JingDongSubsidy(stop), //京东金贴
        JingDongGetCash(stop), //京东领现金
        JingDongShake(stop), //京东摇一摇
        JDSecKilling(stop), //京东秒杀
        JRLuckyLottery(stop), //金融抽奖顶部签到
        JingDongBuyCar(stop, '5fc3c1f2e91f46f09ab2b722e10d92bf'), //京东汽车
        JingRongDoll(stop, 'JRDoll', '京东金融-签壹', '4D25A6F482'),
        JingRongDoll(stop, 'JRTwoDoll', '京东金融-签贰', '3A3E839252'),
        JingRongDoll(stop, 'JRThreeDoll', '京东金融-签叁', '69F5EC743C'),
        JingRongDoll(stop, 'JRFourDoll', '京东金融-签肆', '30C4F86264'),
        JingRongDoll(stop, 'JRFiveDoll', '京东金融-签伍', '1D06AA3B0F')
      ]);
      await Promise.all([
        JDUserSignPre(stop, 'JDClothing', '京东商城-服饰', '4RBT3H9jmgYg1k2kBnHF8NAHm7m8'), //京东服饰
        JDUserSignPre(stop, 'JDUnderwear', '京东商城-内衣', '4PgpL1xqPSW1sVXCJ3xopDbB1f69'), //京东内衣馆
        JDUserSignPre(stop, 'JDShoes', '京东商城-鞋靴', '4RXyb1W4Y986LJW8ToqMK14BdTD'), //京东鞋靴
        JDUserSignPre(stop, 'JDCalendar', '京东日历-翻牌', '36V2Qw59VPNsuLxY84vCFtxFzrFs'), //京东日历翻牌
        JDUserSignPre(stop, 'JDChild', '京东商城-童装', '3Af6mZNcf5m795T8dtDVfDwWVNhJ'), //京东童装馆
        JDUserSignPre(stop, 'JDBaby', '京东商城-母婴', '3BbAVGQPDd6vTyHYjmAutXrKAos6'), //京东母婴馆
        JDUserSignPre(stop, 'JD3C', '京东商城-数码', '4SWjnZSCTHPYjE5T7j35rxxuMTb6'), //京东数码电器馆
        JDUserSignPre(stop, 'JDSubsidy', '京东晚市-补贴', 'xK148m4kWj5hBcTPuJUNNXH3AkJ'), //京东晚市补贴金
        JDUserSignPre(stop, 'JDDrug', '京东商城-医药', '3tqTG5sF1xCUyC6vgEF5CLCxGn7w'), //京东医药馆
        JDUserSignPre(stop, 'JDWomen', '京东商城-女装', 'DpSh7ma8JV7QAxSE2gJNro8Q2h9'), //京东女装馆
        JDUserSignPre(stop, 'JDFineWine', '京东商城-酒饮', 'zGwAUzL3pVGjptBBGeYfpKjYdtX'), //京东酒饮馆
        JDUserSignPre(stop, 'JDBook', '京东商城-图书', '3SC6rw5iBg66qrXPGmZMqFDwcyXi'), //京东图书
        JingRongDoll(stop, 'JTDouble', '京东金贴-双签', '1DF13833F7'), //京东金融 金贴双签
        JingRongDoll(stop, 'XJDouble', '金融现金-双签', 'F68B2C3E71', '', '', '', 'xianjin') //京东金融 现金双签
      ]);
      await Promise.all([
        JDUserSignPre(stop, 'JDSuitcase', '京东商城-箱包', 'ZrH7gGAcEkY2gH8wXqyAPoQgk6t'), //京东箱包馆
        JDUserSignPre(stop, 'JDSchool', '京东商城-校园', '4812pn2PAcUyfNdWr7Cvpww5MCyW'), //京东校园
        JDUserSignPre(stop, 'JDHealth', '京东商城-健康', 'w2oeK5yLdHqHvwef7SMMy4PL8LF'), //京东健康
        JDUserSignPre(stop, 'JDPet', '京东商城-宠物', '37ta5sh5ocrMZF3Fz5UMJbTsL42'), //京东宠物馆
        JDUserSignPre(stop, 'JDShand', '京东拍拍-二手', '3S28janPLYmtFxypu37AYAGgivfp'), //京东拍拍二手
        JDUserSignPre(stop, 'JDClean', '京东商城-清洁', '2Tjm6ay1ZbZ3v7UbriTj6kHy9dn6'), //京东清洁馆
        JDUserSignPre(stop, 'JDCare', '京东商城-个护', 'NJ1kd1PJWhwvhtim73VPsD1HwY3'), //京东个人护理馆
        JDUserSignPre(stop, 'JDJewels', '京东商城-珠宝', 'zHUHpTHNTaztSRfNBFNVZscyFZU'), //京东珠宝馆
        JDUserSignPre(stop, 'JDClocks', '京东商城-钟表', '2BcJPCVVzMEtMUynXkPscCSsx68W'), //京东钟表馆
        JDUserSignPre(stop, 'JDMakeup', '京东商城-美妆', '2smCxzLNuam5L14zNJHYu43ovbAP'), //京东美妆馆
        JDUserSignPre(stop, 'JDVege', '京东商城-菜场', 'Wcu2LVCFMkBP3HraRvb7pgSpt64') //京东菜场
      ]);
      await JingDongSpeedUp(stop); //京东天天加速
      await JingRongDoll(stop, 'JDDouble', '金融京豆-双签', 'F68B2C3E71', '', '', '', 'jingdou'); //京东金融 京豆双签
      break;
    default:
      await JingDongBean(stop); //京东京豆
      await JDTakeaLook(stop); //京东发现-看一看
      await JingDongStore(stop); //京东超市
      await JingDongWebcasts(stop); //京东直播
      await JingRongBean(stop); //金融简单赚钱
      await JingRongSteel(stop); //金融钢镚
      await JingDongTurn(stop); //京东转盘
      await JDFlashSale(stop); //京东闪购
      await JDOverseas(stop); //京东国际
      await JingDongCash(stop); //京东现金红包
      await JDMagicCube(stop, 2); //京东小魔方
      await JingDongGetCash(stop); //京东领现金
      await JingDongSubsidy(stop); //京东金贴
      await JingDongShake(stop); //京东摇一摇
      await JDSecKilling(stop); //京东秒杀
      await JRLuckyLottery(stop); //金融抽奖顶部签到
      await JingDongBuyCar(stop, '5fc3c1f2e91f46f09ab2b722e10d92bf'); //京东汽车
      await JingRongDoll(stop, 'JRTwoDoll', '京东金融-签贰', '3A3E839252');
      await JingRongDoll(stop, 'JRThreeDoll', '京东金融-签叁', '69F5EC743C');
      await JingRongDoll(stop, 'JRFourDoll', '京东金融-签肆', '30C4F86264');
      await JingRongDoll(stop, 'JRFiveDoll', '京东金融-签伍', '1D06AA3B0F');
      await JingRongDoll(stop, 'JRDoll', '京东金融-签壹', '4D25A6F482');
      await JingRongDoll(stop, 'XJDouble', '金融现金-双签', 'F68B2C3E71', '', '', '', 'xianjin'); //京东金融 现金双签
      await JingRongDoll(stop, 'JTDouble', '京东金贴-双签', '1DF13833F7'); //京东金融 金贴双签
      await JDUserSignPre(stop, 'JDUnderwear', '京东商城-内衣', '4PgpL1xqPSW1sVXCJ3xopDbB1f69'); //京东内衣馆
      await JDUserSignPre(stop, 'JDSuitcase', '京东商城-箱包', 'ZrH7gGAcEkY2gH8wXqyAPoQgk6t'); //京东箱包馆
      await JDUserSignPre(stop, 'JDClothing', '京东商城-服饰', '4RBT3H9jmgYg1k2kBnHF8NAHm7m8'); //京东服饰
      await JDUserSignPre(stop, 'JDSchool', '京东商城-校园', '4812pn2PAcUyfNdWr7Cvpww5MCyW'); //京东校园
      await JDUserSignPre(stop, 'JDHealth', '京东商城-健康', 'w2oeK5yLdHqHvwef7SMMy4PL8LF'); //京东健康
      await JDUserSignPre(stop, 'JDShoes', '京东商城-鞋靴', '4RXyb1W4Y986LJW8ToqMK14BdTD'); //京东鞋靴
      await JDUserSignPre(stop, 'JDCalendar', '京东日历-翻牌', '36V2Qw59VPNsuLxY84vCFtxFzrFs'); //京东日历翻牌
      await JDUserSignPre(stop, 'JDChild', '京东商城-童装', '3Af6mZNcf5m795T8dtDVfDwWVNhJ'); //京东童装馆
      await JDUserSignPre(stop, 'JDBaby', '京东商城-母婴', '3BbAVGQPDd6vTyHYjmAutXrKAos6'); //京东母婴馆
      await JDUserSignPre(stop, 'JD3C', '京东商城-数码', '4SWjnZSCTHPYjE5T7j35rxxuMTb6'); //京东数码电器馆
      await JDUserSignPre(stop, 'JDSubsidy', '京东晚市-补贴', 'xK148m4kWj5hBcTPuJUNNXH3AkJ'); //京东晚市补贴金
      await JDUserSignPre(stop, 'JDClocks', '京东商城-钟表', '2BcJPCVVzMEtMUynXkPscCSsx68W'); //京东钟表馆
      await JDUserSignPre(stop, 'JDDrug', '京东商城-医药', '3tqTG5sF1xCUyC6vgEF5CLCxGn7w'); //京东医药馆
      await JDUserSignPre(stop, 'JDWomen', '京东商城-女装', 'DpSh7ma8JV7QAxSE2gJNro8Q2h9'); //京东女装馆
      await JDUserSignPre(stop, 'JDPet', '京东商城-宠物', '37ta5sh5ocrMZF3Fz5UMJbTsL42'); //京东宠物馆
      await JDUserSignPre(stop, 'JDBook', '京东商城-图书', '3SC6rw5iBg66qrXPGmZMqFDwcyXi'); //京东图书
      await JDUserSignPre(stop, 'JDShand', '京东拍拍-二手', '3S28janPLYmtFxypu37AYAGgivfp'); //京东拍拍二手
      await JDUserSignPre(stop, 'JDMakeup', '京东商城-美妆', '2smCxzLNuam5L14zNJHYu43ovbAP'); //京东美妆馆
      await JDUserSignPre(stop, 'JDFineWine', '京东商城-酒饮', 'zGwAUzL3pVGjptBBGeYfpKjYdtX'); //京东酒饮馆
      await JDUserSignPre(stop, 'JDVege', '京东商城-菜场', 'Wcu2LVCFMkBP3HraRvb7pgSpt64'); //京东菜场
      await JDUserSignPre(stop, 'JDClean', '京东商城-清洁', '2Tjm6ay1ZbZ3v7UbriTj6kHy9dn6'); //京东清洁馆
      await JDUserSignPre(stop, 'JDCare', '京东商城-个护', 'NJ1kd1PJWhwvhtim73VPsD1HwY3'); //京东个人护理馆
      await JDUserSignPre(stop, 'JDJewels', '京东商城-珠宝', 'zHUHpTHNTaztSRfNBFNVZscyFZU'); //京东珠宝馆
      await JingRongDoll(stop, 'JDDouble', '金融京豆-双签', 'F68B2C3E71', '', '', '', 'jingdou'); //京东金融 京豆双签
      await JingDongSpeedUp(stop); //京东天天加速
      break;
  }
  await Promise.all([
    TotalSteel(), //总钢镚查询
    TotalCash(), //总红包查询
    TotalBean(), //总京豆查询
    TotalSubsidy(), //总金贴查询
    TotalMoney() //总现金查询
  ]);
  await notify(); //通知模块
}

function notify() {
  return new Promise(resolve => {
    try {
      var bean = 0;
      var steel = 0;
      var cash = 0;
      var money = 0;
      var subsidy = 0;
      var success = 0;
      var fail = 0;
      var err = 0;
      var notify = '';
      for (var i in merge) {
        bean += merge[i].bean ? Number(merge[i].bean) : 0
        steel += merge[i].steel ? Number(merge[i].steel) : 0
        cash += merge[i].Cash ? Number(merge[i].Cash) : 0
        money += merge[i].Money ? Number(merge[i].Money) : 0
        subsidy += merge[i].subsidy ? Number(merge[i].subsidy) : 0
        success += merge[i].success ? Number(merge[i].success) : 0
        fail += merge[i].fail ? Number(merge[i].fail) : 0
        err += merge[i].error ? Number(merge[i].error) : 0
        notify += merge[i].notify ? "\n" + merge[i].notify : ""
      }
      var Cash = merge.TotalCash && merge.TotalCash.TCash ? `${merge.TotalCash.TCash}红包` : ""
      var Steel = merge.TotalSteel && merge.TotalSteel.TSteel ? `${merge.TotalSteel.TSteel}钢镚` : ``
      var beans = merge.TotalBean && merge.TotalBean.Qbear ? `${merge.TotalBean.Qbear}京豆${Steel?`, `:``}` : ""
      var Money = merge.TotalMoney && merge.TotalMoney.TMoney ? `${merge.TotalMoney.TMoney}现金${Cash?`, `:``}` : ""
      var Subsidy = merge.TotalSubsidy && merge.TotalSubsidy.TSubsidy ? `${merge.TotalSubsidy.TSubsidy}金贴${Money||Cash?", ":""}` : ""
      var Tbean = bean ? `${bean.toFixed(0)}京豆${steel?", ":""}` : ""
      var TSteel = steel ? `${steel.toFixed(2)}钢镚` : ""
      var TCash = cash ? `${cash.toFixed(2)}红包${subsidy||money?", ":""}` : ""
      var TSubsidy = subsidy ? `${subsidy.toFixed(2)}金贴${money?", ":""}` : ""
      var TMoney = money ? `${money.toFixed(2)}现金` : ""
      var Ts = success ? `成功${success}个${fail||err?`, `:``}` : ``
      var Tf = fail ? `失败${fail}个${err?`, `:``}` : ``
      var Te = err ? `错误${err}个` : ``
      var one = `【签到概览】:  ${Ts+Tf+Te}${Ts||Tf||Te?`\n`:`获取失败\n`}`
      var two = Tbean || TSteel ? `【签到奖励】:  ${Tbean+TSteel}\n` : ``
      var three = TCash || TSubsidy || TMoney ? `【其他奖励】:  ${TCash+TSubsidy+TMoney}\n` : ``
      var four = `【账号总计】:  ${beans+Steel}${beans||Steel?`\n`:`获取失败\n`}`
      var five = `【其他总计】:  ${Subsidy+Money+Cash}${Subsidy||Money||Cash?`\n`:`获取失败\n`}`
      var DName = merge.TotalBean && merge.TotalBean.nickname ? merge.TotalBean.nickname : "获取失败"
      var Name = add ? DualAccount ? `【签到号一】:  ${DName}\n` : `【签到号二】:  ${DName}\n` : ""
      const disables = $nobyda.read("JD_DailyBonusDisables")
      const amount = disables ? disables.split(",").length : 0
      const disa = !notify || amount ? `【温馨提示】:  检测到${$nobyda.disable?`上次执行意外崩溃, `:``}已禁用${notify?`${amount}个`:`所有`}接口, 如需开启请前往BoxJs或查看脚本内第99行注释.\n` : ``
      $nobyda.notify("", "", Name + one + two + three + four + five + disa + notify, {
        'media-url': $nobyda.headUrl || 'https://cdn.jsdelivr.net/gh/Orz-3/task@master/jd.png'
      });
      $nobyda.headUrl = null;
      if ($nobyda.isJSBox) {
        if (add && DualAccount) {
          Shortcut = Name + one + two + three + "\n"
        } else if (!add && DualAccount) {
          $intents.finish(Name + one + two + three + four + five + notify)
        } else if (typeof Shortcut != "undefined") {
          $intents.finish(Shortcut + Name + one + two + three)
        }
      }
      if (DualAccount) {
        double();
      } else {
        $nobyda.time();
        $nobyda.done();
      }
    } catch (eor) {
      $nobyda.notify("通知模块 " + eor.name + "‼️", JSON.stringify(eor), eor.message)
    } finally {
      resolve()
    }
  });
}

function ReadCookie() {
  DualAccount = true;
  const EnvInfo = $nobyda.isJSBox ? "JD_Cookie" : "CookieJD"
  const EnvInfo2 = $nobyda.isJSBox ? "JD_Cookie2" : "CookieJD2"
  if (DeleteCookie) {
    if ($nobyda.read(EnvInfo) || $nobyda.read(EnvInfo2)) {
      $nobyda.write("", EnvInfo)
      $nobyda.write("", EnvInfo2)
      $nobyda.notify("京东Cookie清除成功 !", "", '请手动关闭脚本内"DeleteCookie"选项')
      $nobyda.done()
      return
    }
    $nobyda.notify("脚本终止", "", '未关闭脚本内"DeleteCookie"选项 ‼️')
    $nobyda.done()
    return
  } else if ($nobyda.isRequest) {
    GetCookie()
    return
  }
  if (Key || $nobyda.read(EnvInfo)) {
    if ($nobyda.isJSBox || $nobyda.isNode) {
      if (Key) $nobyda.write(Key, EnvInfo);
      if (DualKey) $nobyda.write(DualKey, EnvInfo2);
    }
    add = DualKey || $nobyda.read(EnvInfo2) ? true : false
    KEY = Key ? Key : $nobyda.read(EnvInfo)
    out = parseInt($nobyda.read("JD_DailyBonusTimeOut")) || out
    stop = parseInt($nobyda.read("JD_DailyBonusDelay")) || stop
    boxdis = $nobyda.read("JD_Crash_disable") === "false" || $nobyda.isNode || $nobyda.isJSBox ? false : boxdis
    LogDetails = $nobyda.read("JD_DailyBonusLog") === "true" || LogDetails
    ReDis = ReDis ? $nobyda.write("", "JD_DailyBonusDisables") : ""
    all()
  } else {
    $nobyda.notify("京东签到", "", "脚本终止, 未获取Cookie ‼️")
    $nobyda.done()
  }
}

function double() {
  add = true
  DualAccount = false
  if (DualKey || $nobyda.read($nobyda.isJSBox ? "JD_Cookie2" : "CookieJD2")) {
    KEY = DualKey ? DualKey : $nobyda.read($nobyda.isJSBox ? "JD_Cookie2" : "CookieJD2")
    all()
  } else {
    $nobyda.time();
    $nobyda.done();
  }
}

function JingDongBean(s) {
  merge.JDBean = {};
  return new Promise(resolve => {
    if (disable("JDBean")) return resolve()
    setTimeout(() => {
      const JDBUrl = {
        url: 'https://api.m.jd.com/client.action',
        headers: {
          Cookie: KEY
        },
        body: 'functionId=signBeanIndex&appid=ld'
      };
      $nobyda.post(JDBUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const cc = JSON.parse(data)
            const Details = LogDetails ? "response:\n" + data : '';
            if (cc.code == 3) {
              console.log("\n" + "京东商城-京豆Cookie失效 " + Details)
              merge.JDBean.notify = "京东商城-京豆: 失败, 原因: Cookie失效‼️"
              merge.JDBean.fail = 1
            } else if (data.match(/跳转至拼图/)) {
              merge.JDBean.notify = "京东商城-京豆: 失败, 需要拼图验证 ⚠️"
              merge.JDBean.fail = 1
            } else if (data.match(/\"status\":\"?1\"?/)) {
              console.log("\n" + "京东商城-京豆签到成功 " + Details)
              if (data.match(/dailyAward/)) {
                merge.JDBean.notify = "京东商城-京豆: 成功, 明细: " + cc.data.dailyAward.beanAward.beanCount + "京豆 🐶"
                merge.JDBean.bean = cc.data.dailyAward.beanAward.beanCount
              } else if (data.match(/continuityAward/)) {
                merge.JDBean.notify = "京东商城-京豆: 成功, 明细: " + cc.data.continuityAward.beanAward.beanCount + "京豆 🐶"
                merge.JDBean.bean = cc.data.continuityAward.beanAward.beanCount
              } else if (data.match(/新人签到/)) {
                const quantity = data.match(/beanCount\":\"(\d+)\".+今天/)
                merge.JDBean.bean = quantity ? quantity[1] : 0
                merge.JDBean.notify = "京东商城-京豆: 成功, 明细: " + (quantity ? quantity[1] : "无") + "京豆 🐶"
              } else {
                merge.JDBean.notify = "京东商城-京豆: 成功, 明细: 无京豆 🐶"
              }
              merge.JDBean.success = 1
            } else {
              merge.JDBean.fail = 1
              console.log("\n" + "京东商城-京豆签到失败 " + Details)
              if (data.match(/(已签到|新人签到)/)) {
                merge.JDBean.notify = "京东商城-京豆: 失败, 原因: 已签过 ⚠️"
              } else if (data.match(/人数较多|S101/)) {
                merge.JDBean.notify = "京东商城-京豆: 失败, 签到人数较多 ⚠️"
              } else {
                merge.JDBean.notify = "京东商城-京豆: 失败, 原因: 未知 ⚠️"
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-京豆", "JDBean", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingDongTurn(s) {
  merge.JDTurn = {}, merge.JDTurn.notify = "", merge.JDTurn.success = 0, merge.JDTurn.bean = 0;
  return new Promise((resolve, reject) => {
    if (disable("JDTurn")) return reject()
    const JDTUrl = {
      url: 'https://api.m.jd.com/client.action?functionId=wheelSurfIndex&body=%7B%22actId%22%3A%22jgpqtzjhvaoym%22%2C%22appSource%22%3A%22jdhome%22%7D&appid=ld',
      headers: {
        Cookie: KEY,
      }
    };
    $nobyda.get(JDTUrl, async function(error, response, data) {
      try {
        if (error) {
          throw new Error(error)
        } else {
          const cc = JSON.parse(data).data.lotteryCode
          const Details = LogDetails ? "response:\n" + data : '';
          if (cc) {
            console.log("\n" + "京东商城-转盘查询成功 " + Details)
            return resolve(cc)
          } else {
            merge.JDTurn.notify = "京东商城-转盘: 失败, 原因: 查询错误 ⚠️"
            merge.JDTurn.fail = 1
            console.log("\n" + "京东商城-转盘查询失败 " + Details)
          }
        }
      } catch (eor) {
        $nobyda.AnError("京东转盘-查询", "JDTurn", eor, response, data)
      } finally {
        reject()
      }
    })
    if (out) setTimeout(reject, out + s)
  }).then(data => {
    return JingDongTurnSign(s, data);
  }, () => {});
}

function JingDongTurnSign(s, code) {
  return new Promise(resolve => {
    setTimeout(() => {
      const JDTUrl = {
        url: `https://api.m.jd.com/client.action?functionId=lotteryDraw&body=%7B%22actId%22%3A%22jgpqtzjhvaoym%22%2C%22appSource%22%3A%22jdhome%22%2C%22lotteryCode%22%3A%22${code}%22%7D&appid=ld`,
        headers: {
          Cookie: KEY,
        }
      };
      $nobyda.get(JDTUrl, async function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const cc = JSON.parse(data)
            const Details = LogDetails ? "response:\n" + data : '';
            const also = merge.JDTurn.notify ? true : false
            if (cc.code == 3) {
              console.log("\n" + "京东转盘Cookie失效 " + Details)
              merge.JDTurn.notify = "京东商城-转盘: 失败, 原因: Cookie失效‼️"
              merge.JDTurn.fail = 1
            } else if (data.match(/(\"T216\"|活动结束)/)) {
              merge.JDTurn.notify = "京东商城-转盘: 失败, 原因: 活动结束 ⚠️"
              merge.JDTurn.fail = 1
            } else if (data.match(/(京豆|\"910582\")/)) {
              console.log("\n" + "京东商城-转盘签到成功 " + Details)
              merge.JDTurn.bean += Number(cc.data.prizeSendNumber) || 0
              merge.JDTurn.notify += `${also?`\n`:``}京东商城-转盘: ${also?`多次`:`成功`}, 明细: ${cc.data.prizeSendNumber||`无`}京豆 🐶`
              merge.JDTurn.success += 1
              if (cc.data.chances != "0") {
                await JingDongTurnSign(2000, code)
              }
            } else if (data.match(/未中奖/)) {
              merge.JDTurn.notify += `${also?`\n`:``}京东商城-转盘: ${also?`多次`:`成功`}, 状态: 未中奖 🐶`
              merge.JDTurn.success += 1
              if (cc.data.chances != "0") {
                await JingDongTurnSign(2000, code)
              }
            } else {
              console.log("\n" + "京东商城-转盘签到失败 " + Details)
              merge.JDTurn.fail = 1
              if (data.match(/(T215|次数为0)/)) {
                merge.JDTurn.notify = "京东商城-转盘: 失败, 原因: 已转过 ⚠️"
              } else if (data.match(/(T210|密码)/)) {
                merge.JDTurn.notify = "京东商城-转盘: 失败, 原因: 无支付密码 ⚠️"
              } else {
                merge.JDTurn.notify += `${also?`\n`:``}京东商城-转盘: 失败, 原因: 未知 ⚠️${also?` (多次)`:``}`
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-转盘", "JDTurn", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingRongBean(s) {
  merge.JRBean = {};
  return new Promise(resolve => {
    if (disable("JRBean")) return resolve()
    setTimeout(() => {
      const login = {
        url: 'https://ms.jr.jd.com/gw/generic/zc/h5/m/queryOpenScreenReward',
        headers: {
          Cookie: KEY
        },
        body: "reqData=%7B%22channelCode%22%3A%22ZHUANQIAN%22%2C%22clientType%22%3A%22sms%22%2C%22clientVersion%22%3A%2211.0%22%7D"
      };
      $nobyda.post(login, async function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            if (data.match(/\"login\":true/)) {
              console.log("\n" + "京东金融-金贴查询成功 " + Details)
              const cc = JSON.parse(data)
              if (cc.resultData.data.reward === false) {
                await JRBeanCheckin(s, cc.resultData.data.rewardAmount);
              } else {
                const tp = cc.resultData.data.reward === true
                merge.JRBean.notify = `京东金融-金贴: 失败, 原因: ${tp?`已签过`:`未知`} ⚠️`
                merge.JRBean.fail = 1
              }
            } else {
              console.log("\n" + "京东金融-金贴查询失败 " + Details)
              const lt = data.match(/\"login\":false/)
              merge.JRBean.fail = 1
              merge.JRBean.notify = `京东金融-金贴: 失败, 原因: ${lt?`Cookie失效‼️`:`未知 ⚠️`}`
            }
          }
        } catch (eor) {
          $nobyda.AnError("金融金贴-查询", "JRBean", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JRBeanCheckin(s, amount) {
  return new Promise(resolve => {
    setTimeout(() => {
      const JRBUrl = {
        url: 'https://ms.jr.jd.com/gw/generic/zc/h5/m/openScreenReward',
        headers: {
          Cookie: KEY
        },
        body: "reqData=%7B%22channelCode%22%3A%22ZHUANQIAN%22%2C%22clientType%22%3A%22sms%22%2C%22clientVersion%22%3A%2211.0%22%7D"
      };
      $nobyda.post(JRBUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            if (data.match(/\"rewardCode\":\"00000\"/)) {
              console.log("\n" + "京东金融-金贴签到成功 " + Details)
              merge.JRBean.subsidy = amount || 0
              merge.JRBean.notify = `京东金融-金贴: 成功, 明细: ${amount || `无`}金贴 💰`
              merge.JRBean.success = 1
            } else {
              console.log("\n" + "京东金融-金贴签到失败 " + Details)
              merge.JRBean.fail = 1
              if (data.match(/发放失败|70111|10000|60203/)) {
                merge.JRBean.notify = "京东金融-金贴: 失败, 原因: 已签过 ⚠️"
              } else {
                const UnType = data.match(/\"resultCode\":3|请先登录/)
                merge.JRBean.notify = `京东金融-金贴: 失败, 原因: ${UnType?`Cookie失效‼️`:`未知 ⚠️`}`
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东金融-金贴", "JRBean", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingRongSteel(s) {
  merge.JRSteel = {};
  return new Promise(resolve => {
    if (disable("JRSteel")) return resolve()
    setTimeout(() => {
      const JRSUrl = {
        url: 'https://ms.jr.jd.com/gw/generic/hy/h5/m/signIn1',
        headers: {
          Cookie: KEY
        },
        body: "reqData=%7B%22channelSource%22%3A%22JRAPP6.0%22%2C%22riskDeviceParam%22%3A%22%7B%7D%22%7D"
      };
      $nobyda.post(JRSUrl, function(error, response, data) {
        try {
          if (error) throw new Error(error)
          const cc = JSON.parse(data)
          const Details = LogDetails ? "response:\n" + data : '';
          if (data.match(/\"resBusiCode\":0/)) {
            console.log("\n" + "京东金融-钢镚签到成功 " + Details)
            const leng = cc.resultData.resBusiData.actualTotalRewardsValue
            const spare = cc.resultData.resBusiData.baseReward
            merge.JRSteel.steel = leng ? leng > 9 ? `0.${leng}` : `0.0${leng}` : spare ? spare : 0
            merge.JRSteel.notify = `京东金融-钢镚: 成功, 明细: ${merge.JRSteel.steel || `无`}钢镚 💰`
            merge.JRSteel.success = 1
          } else {
            console.log("\n" + "京东金融-钢镚签到失败 " + Details)
            merge.JRSteel.fail = 1
            if (data.match(/已经领取|\"resBusiCode\":15/)) {
              merge.JRSteel.notify = "京东金融-钢镚: 失败, 原因: 已签过 ⚠️"
            } else if (data.match(/未实名/)) {
              merge.JRSteel.notify = "京东金融-钢镚: 失败, 账号未实名 ⚠️"
            } else if (data.match(/(\"resultCode\":3|请先登录)/)) {
              merge.JRSteel.notify = "京东金融-钢镚: 失败, 原因: Cookie失效‼️"
            } else {
              merge.JRSteel.notify = "京东金融-钢镚: 失败, 原因: 未知 ⚠️"
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东金融-钢镚", "JRSteel", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingDongShake(s) {
  if (!merge.JDShake) merge.JDShake = {}, merge.JDShake.success = 0, merge.JDShake.bean = 0, merge.JDShake.notify = '';
  return new Promise(resolve => {
    if (disable("JDShake")) return resolve()
    setTimeout(() => {
      const JDSh = {
        url: 'https://api.m.jd.com/client.action?appid=vip_h5&functionId=vvipclub_shaking',
        headers: {
          Cookie: KEY,
        }
      };
      $nobyda.get(JDSh, async function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            const cc = JSON.parse(data)
            const also = merge.JDShake.notify ? true : false
            if (data.match(/prize/)) {
              console.log("\n" + "京东商城-摇一摇签到成功 " + Details)
              merge.JDShake.success += 1
              if (cc.data.prizeBean) {
                merge.JDShake.bean += cc.data.prizeBean.count || 0
                merge.JDShake.notify += `${also?`\n`:``}京东商城-摇摇: ${also?`多次`:`成功`}, 明细: ${merge.JDShake.bean || `无`}京豆 🐶`
              } else if (cc.data.prizeCoupon) {
                merge.JDShake.notify += `${also?`\n`:``}京东商城-摇摇: ${also?`多次, `:``}获得满${cc.data.prizeCoupon.quota}减${cc.data.prizeCoupon.discount}优惠券→ ${cc.data.prizeCoupon.limitStr}`
              } else {
                merge.JDShake.notify += `${also?`\n`:``}京东商城-摇摇: 成功, 明细: 未知 ⚠️${also?` (多次)`:``}`
              }
              if (cc.data.luckyBox.freeTimes != 0) {
                await JingDongShake(s)
              }
            } else {
              console.log("\n" + "京东商城-摇一摇签到失败 " + Details)
              if (data.match(/true/)) {
                merge.JDShake.notify += `${also?`\n`:``}京东商城-摇摇: 成功, 明细: 无奖励 🐶${also?` (多次)`:``}`
                merge.JDShake.success += 1
                if (cc.data.luckyBox.freeTimes != 0) {
                  await JingDongShake(s)
                }
              } else {
                merge.JDShake.fail = 1
                if (data.match(/(无免费|8000005|9000005)/)) {
                  merge.JDShake.notify = "京东商城-摇摇: 失败, 原因: 已摇过 ⚠️"
                } else if (data.match(/(未登录|101)/)) {
                  merge.JDShake.notify = "京东商城-摇摇: 失败, 原因: Cookie失效‼️"
                } else {
                  merge.JDShake.notify += `${also?`\n`:``}京东商城-摇摇: 失败, 原因: 未知 ⚠️${also?` (多次)`:``}`
                }
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-摇摇", "JDShake", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JDUserSignPre(s, key, title, ac) {
  merge[key] = {};
  if ($nobyda.isJSBox) {
    return JDUserSignPre2(s, key, title, ac);
  } else {
    return JDUserSignPre1(s, key, title, ac);
  }
}

function JDUserSignPre1(s, key, title, acData, ask) {
  return new Promise((resolve, reject) => {
    if (disable(key, title, 1)) return reject()
    const JDUrl = {
      url: 'https://api.m.jd.com/?client=wh5&functionId=qryH5BabelFloors',
      headers: {
        Cookie: KEY
      },
      opts: {
        'filter': 'try{var od=JSON.parse(body);var params=(od.floatLayerList||[]).filter(o=>o.params&&o.params.match(/enActK/)).map(o=>o.params).pop()||(od.floorList||[]).filter(o=>o.template=="signIn"&&o.signInfos&&o.signInfos.params&&o.signInfos.params.match(/enActK/)).map(o=>o.signInfos&&o.signInfos.params).pop();var tId=(od.floorList||[]).filter(o=>o.boardParams&&o.boardParams.turnTableId).map(o=>o.boardParams.turnTableId).pop();var page=od.paginationFlrs;return JSON.stringify({qxAct:params||null,qxTid:tId||null,qxPage:page||null})}catch(e){return `=> 过滤器发生错误: ${e.message}`}'
      },
      body: `body=${encodeURIComponent(`{"activityId":"${acData}"${ask?`,"paginationParam":"2","paginationFlrs":"${ask}"`:``}}`)}`
    };
    $nobyda.post(JDUrl, async function(error, response, data) {
      try {
        if (error) {
          throw new Error(error)
        } else {
          const od = JSON.parse(data || '{}');
          const turnTableId = od.qxTid || (od.floorList || []).filter(o => o.boardParams && o.boardParams.turnTableId).map(o => o.boardParams.turnTableId).pop();
          const page = od.qxPage || od.paginationFlrs;
          if (data.match(/enActK/)) { // 含有签到活动数据
            let params = od.qxAct || (od.floatLayerList || []).filter(o => o.params && o.params.match(/enActK/)).map(o => o.params).pop()
            if (!params) { // 第一处找到签到所需数据
              // floatLayerList未找到签到所需数据，从floorList中查找
              let signInfo = (od.floorList || []).filter(o => o.template == 'signIn' && o.signInfos && o.signInfos.params && o.signInfos.params.match(/enActK/))
                .map(o => o.signInfos).pop();
              if (signInfo) {
                if (signInfo.signStat == '1') {
                  console.log(`\n${title}重复签到`)
                  merge[key].notify = `${title}: 失败, 原因: 已签过 ⚠️`
                  merge[key].fail = 1
                } else {
                  params = signInfo.params;
                }
              } else {
                merge[key].notify = `${title}: 失败, 活动查找异常 ⚠️`
                merge[key].fail = 1
              }
            }
            if (params) {
              return resolve({
                params: params
              }); // 执行签到处理
            }
          } else if (turnTableId) { // 无签到数据, 但含有关注店铺签到
            const boxds = $nobyda.read("JD_Follow_disable") === "false" ? false : true
            if (boxds) {
              console.log(`\n${title}关注店铺`)
              return resolve(parseInt(turnTableId))
            } else {
              merge[key].notify = `${title}: 失败, 需要关注店铺 ⚠️`
              merge[key].fail = 1
            }
          } else if (page && !ask) { // 无签到数据, 尝试带参查询
            const boxds = $nobyda.read("JD_Retry_disable") === "false" ? false : true
            if (boxds) {
              console.log(`\n${title}二次查询`)
              return resolve(page)
            } else {
              merge[key].notify = `${title}: 失败, 请尝试开启增强 ⚠️`
              merge[key].fail = 1
            }
          } else {
            merge[key].notify = `${title}: 失败, ${!data ? `需要手动执行` : `不含活动数据`} ⚠️`
            merge[key].fail = 1
          }
        }
        reject()
      } catch (eor) {
        $nobyda.AnError(title, key, eor, response, data)
        reject()
      }
    })
    if (out) setTimeout(reject, out + s)
  }).then(data => {
    disable(key, title, 2)
    if (typeof(data) == "object") return JDUserSign1(s, key, title, encodeURIComponent(JSON.stringify(data)));
    if (typeof(data) == "number") return JDUserSign2(s, key, title, data);
    if (typeof(data) == "string") return JDUserSignPre1(s, key, title, acData, data);
  }, () => disable(key, title, 2))
}

function JDUserSignPre2(s, key, title, acData) {
  return new Promise((resolve, reject) => {
    if (disable(key, title, 1)) return reject()
    const JDUrl = {
      url: `https://pro.m.jd.com/mall/active/${acData}/index.html`,
      headers: {
        Cookie: KEY,
      }
    };
    $nobyda.get(JDUrl, async function(error, response, data) {
      try {
        if (error) {
          throw new Error(error)
        } else {
          const act = data.match(/\"params\":\"\{\\\"enActK.+?\\\"\}\"/)
          const turnTable = data.match(/\"turnTableId\":\"(\d+)\"/)
          const page = data.match(/\"paginationFlrs\":\"(\[\[.+?\]\])\"/)
          if (act) { // 含有签到活动数据
            return resolve(act)
          } else if (turnTable) { // 无签到数据, 但含有关注店铺签到
            const boxds = $nobyda.read("JD_Follow_disable") === "false" ? false : true
            if (boxds) {
              console.log(`\n${title}关注店铺`)
              return resolve(parseInt(turnTable[1]))
            } else {
              merge[key].notify = `${title}: 失败, 需要关注店铺 ⚠️`
              merge[key].fail = 1
            }
          } else if (page) { // 无签到数据, 尝试带参查询
            const boxds = $nobyda.read("JD_Retry_disable") === "false" ? false : true
            if (boxds) {
              console.log(`\n${title}二次查询`)
              return resolve(page[1])
            } else {
              merge[key].notify = `${title}: 失败, 请尝试开启增强 ⚠️`
              merge[key].fail = 1
            }
          } else {
            merge[key].notify = `${title}: 失败, ${!data ? `需要手动执行` : `不含活动数据`} ⚠️`
            merge[key].fail = 1
          }
        }
        reject()
      } catch (eor) {
        $nobyda.AnError(title, key, eor, response, data)
        reject()
      }
    })
    if (out) setTimeout(reject, out + s)
  }).then(data => {
    disable(key, title, 2)
    if (typeof(data) == "object") return JDUserSign1(s, key, title, encodeURIComponent(`{${data}}`));
    if (typeof(data) == "number") return JDUserSign2(s, key, title, data)
    if (typeof(data) == "string") return JDUserSignPre1(s, key, title, acData, data)
  }, () => disable(key, title, 2))
}

function JDUserSign1(s, key, title, body) {
  return new Promise(resolve => {
    setTimeout(() => {
      const JDUrl = {
        url: 'https://api.m.jd.com/client.action?functionId=userSign',
        headers: {
          Cookie: KEY
        },
        body: `body=${body}&client=wh5`
      };
      $nobyda.post(JDUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? `response:\n${data}` : '';
            if (data.match(/签到成功/)) {
              console.log(`\n${title}签到成功(1)${Details}`)
              if (data.match(/\"text\":\"\d+京豆\"/)) {
                merge[key].bean = data.match(/\"text\":\"(\d+)京豆\"/)[1]
              }
              merge[key].notify = `${title}: 成功, 明细: ${merge[key].bean || '无'}京豆 🐶`
              merge[key].success = 1
            } else {
              console.log(`\n${title}签到失败(1)${Details}`)
              if (data.match(/(已签到|已领取)/)) {
                merge[key].notify = `${title}: 失败, 原因: 已签过 ⚠️`
              } else if (data.match(/(不存在|已结束|未开始)/)) {
                merge[key].notify = `${title}: 失败, 原因: 活动已结束 ⚠️`
              } else if (data.match(/\"code\":\"?3\"?/)) {
                merge[key].notify = `${title}: 失败, 原因: Cookie失效‼️`
              } else {
                const ng = data.match(/\"(errorMessage|subCodeMsg)\":\"(.+?)\"/)
                merge[key].notify = `${title}: 失败, ${ng?ng[2]:`原因: 未知`} ⚠️`
              }
              merge[key].fail = 1
            }
          }
        } catch (eor) {
          $nobyda.AnError(title, key, eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

async function JDUserSign2(s, key, title, tid) {
  await new Promise(resolve => {
    $nobyda.get({
      url: `https://jdjoy.jd.com/api/turncard/channel/detail?turnTableId=${tid}`,
      headers: {
        Cookie: KEY
      }
    }, function(error, response, data) {
      resolve()
    })
    if (out) setTimeout(resolve, out + s)
  });
  return new Promise(resolve => {
    setTimeout(() => {
      const JDUrl = {
        url: 'https://jdjoy.jd.com/api/turncard/channel/sign',
        headers: {
          Cookie: KEY
        },
        body: `turnTableId=${tid}`
      };
      $nobyda.post(JDUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? `response:\n${data}` : '';
            if (data.match(/\"success\":true/)) {
              console.log(`\n${title}签到成功(2)${Details}`)
              if (data.match(/\"jdBeanQuantity\":\d+/)) {
                merge[key].bean = data.match(/\"jdBeanQuantity\":(\d+)/)[1]
              }
              merge[key].notify = `${title}: 成功, 明细: ${merge[key].bean || '无'}京豆 🐶`
              merge[key].success = 1
            } else {
              console.log(`\n${title}签到失败(2)${Details}`)
              if (data.match(/(已经签到|已经领取)/)) {
                merge[key].notify = `${title}: 失败, 原因: 已签过 ⚠️`
              } else if (data.match(/(不存在|已结束|未开始)/)) {
                merge[key].notify = `${title}: 失败, 原因: 活动已结束 ⚠️`
              } else if (data.match(/(没有登录|B0001)/)) {
                merge[key].notify = `${title}: 失败, 原因: Cookie失效‼️`
              } else {
                const ng = data.match(/\"(errorMessage|subCodeMsg)\":\"(.+?)\"/)
                merge[key].notify = `${title}: 失败, ${ng?ng[2]:`原因: 未知`} ⚠️`
              }
              merge[key].fail = 1
            }
          }
        } catch (eor) {
          $nobyda.AnError(title, key, eor, response, data)
        } finally {
          resolve()
        }
      })
    }, 200 + s)
    if (out) setTimeout(resolve, out + s + 200)
  });
}

function JDFlashSale(s) {
  merge.JDFSale = {};
  return new Promise(resolve => {
    if (disable("JDFSale")) return resolve()
    setTimeout(() => {
      const JDPETUrl = {
        url: 'https://api.m.jd.com/client.action?functionId=partitionJdSgin',
        headers: {
          Cookie: KEY
        },
        body: "body=%7B%22version%22%3A%22v2%22%7D&client=apple&clientVersion=9.0.8&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&sign=6768e2cf625427615dd89649dd367d41&st=1597248593305&sv=121"
      };
      $nobyda.post(JDPETUrl, async function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            const cc = JSON.parse(data)
            if (cc.result && cc.result.code == 0) {
              console.log("\n" + "京东商城-闪购签到成功 " + Details)
              merge.JDFSale.bean = cc.result.jdBeanNum || 0
              merge.JDFSale.notify = "京东商城-闪购: 成功, 明细: " + (merge.JDFSale.bean || "无") + "京豆 🐶"
              merge.JDFSale.success = 1
            } else {
              console.log("\n" + "京东商城-闪购签到失败 " + Details)
              if (data.match(/(已签到|已领取|\"2005\")/)) {
                merge.JDFSale.notify = "京东商城-闪购: 失败, 原因: 已签过 ⚠️"
              } else if (data.match(/不存在|已结束|\"2008\"|\"3001\"/)) {
                await FlashSaleDivide(s); //瓜分京豆
                return
              } else if (data.match(/(\"code\":\"3\"|\"1003\")/)) {
                merge.JDFSale.notify = "京东商城-闪购: 失败, 原因: Cookie失效‼️"
              } else {
                const msg = data.match(/\"msg\":\"([\u4e00-\u9fa5].+?)\"/)
                merge.JDFSale.notify = `京东商城-闪购: 失败, ${msg ? msg[1] : `原因: 未知`} ⚠️`
              }
              merge.JDFSale.fail = 1
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-闪购", "JDFSale", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function FlashSaleDivide(s) {
  return new Promise(resolve => {
    setTimeout(() => {
      const Url = {
        url: 'https://api.m.jd.com/client.action?functionId=partitionJdShare',
        headers: {
          Cookie: KEY
        },
        body: "body=%7B%22version%22%3A%22v2%22%7D&client=apple&clientVersion=9.0.8&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&sign=49baa3b3899b02bbf06cdf41fe191986&st=1597682588351&sv=111"
      };
      $nobyda.post(Url, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            const cc = JSON.parse(data)
            if (cc.result.code == 0) {
              merge.JDFSale.success = 1
              merge.JDFSale.bean = cc.result.jdBeanNum || 0
              merge.JDFSale.notify = "京东闪购-瓜分: 成功, 明细: " + (merge.JDFSale.bean || "无") + "京豆 🐶"
              console.log("\n" + "京东闪购-瓜分签到成功 " + Details)
            } else {
              merge.JDFSale.fail = 1
              console.log("\n" + "京东闪购-瓜分签到失败 " + Details)
              if (data.match(/已参与|已领取|\"2006\"/)) {
                merge.JDFSale.notify = "京东闪购-瓜分: 失败, 原因: 已瓜分 ⚠️"
              } else if (data.match(/不存在|已结束|未开始|\"2008\"|\"2012\"/)) {
                merge.JDFSale.notify = "京东闪购-瓜分: 失败, 原因: 活动已结束 ⚠️"
              } else if (data.match(/\"code\":\"1003\"|未获取/)) {
                merge.JDFSale.notify = "京东闪购-瓜分: 失败, 原因: Cookie失效‼️"
              } else {
                const msg = data.match(/\"msg\":\"([\u4e00-\u9fa5].+?)\"/)
                merge.JDFSale.notify = `京东闪购-瓜分: 失败, ${msg ? msg[1] : `原因: 未知`} ⚠️`
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东闪购-瓜分", "JDFSale", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingDongCash(s) {
  merge.JDCash = {};
  return new Promise(resolve => {
    if (disable("JDCash")) return resolve()
    setTimeout(() => {
      const JDCAUrl = {
        url: 'https://api.m.jd.com/client.action?functionId=ccSignInNew',
        headers: {
          Cookie: KEY
        },
        body: "body=%7B%22pageClickKey%22%3A%22CouponCenter%22%2C%22eid%22%3A%22O5X6JYMZTXIEX4VBCBWEM5PTIZV6HXH7M3AI75EABM5GBZYVQKRGQJ5A2PPO5PSELSRMI72SYF4KTCB4NIU6AZQ3O6C3J7ZVEP3RVDFEBKVN2RER2GTQ%22%2C%22shshshfpb%22%3A%22v1%5C%2FzMYRjEWKgYe%2BUiNwEvaVlrHBQGVwqLx4CsS9PH1s0s0Vs9AWk%2B7vr9KSHh3BQd5NTukznDTZnd75xHzonHnw%3D%3D%22%2C%22childActivityUrl%22%3A%22openapp.jdmobile%253a%252f%252fvirtual%253fparams%253d%257b%255c%2522category%255c%2522%253a%255c%2522jump%255c%2522%252c%255c%2522des%255c%2522%253a%255c%2522couponCenter%255c%2522%257d%22%2C%22monitorSource%22%3A%22cc_sign_ios_index_config%22%7D&client=apple&clientVersion=8.5.0&d_brand=apple&d_model=iPhone8%2C2&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&scope=11&screen=1242%2A2208&sign=1cce8f76d53fc6093b45a466e93044da&st=1581084035269&sv=102"
      };
      $nobyda.post(JDCAUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            const cc = JSON.parse(data)
            if (cc.busiCode == "0") {
              console.log("\n" + "京东现金-红包签到成功 " + Details)
              merge.JDCash.success = 1
              merge.JDCash.Cash = cc.result.signResult.signData.amount || 0
              merge.JDCash.notify = `京东现金-红包: 成功, 明细: ${merge.JDCash.Cash || `无`}红包 🧧`
            } else {
              console.log("\n" + "京东现金-红包签到失败 " + Details)
              merge.JDCash.fail = 1
              if (data.match(/(\"busiCode\":\"1002\"|完成签到)/)) {
                merge.JDCash.notify = "京东现金-红包: 失败, 原因: 已签过 ⚠️"
              } else if (data.match(/(不存在|已结束)/)) {
                merge.JDCash.notify = "京东现金-红包: 失败, 原因: 活动已结束 ⚠️"
              } else if (data.match(/(\"busiCode\":\"3\"|未登录)/)) {
                merge.JDCash.notify = "京东现金-红包: 失败, 原因: Cookie失效‼️"
              } else {
                merge.JDCash.notify = "京东现金-红包: 失败, 原因: 未知 ⚠️"
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东现金-红包", "JDCash", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JDMagicCube(s, sign) {
  merge.JDCube = {};
  return new Promise((resolve, reject) => {
    if (disable("JDCube")) return reject()
    const JDUrl = {
      url: `https://api.m.jd.com/client.action?functionId=getNewsInteractionInfo&appid=smfe${sign?`&body=${encodeURIComponent(`{"sign":${sign}}`)}`:``}`,
      headers: {
        Cookie: KEY,
      }
    };
    $nobyda.get(JDUrl, async (error, response, data) => {
      try {
        if (error) throw new Error(error)
        const Details = LogDetails ? "response:\n" + data : '';
        console.log(`\n京东魔方-尝试查询活动(${sign}) ${Details}`)
        if (data.match(/\"interactionId\":\d+/)) {
          resolve({
            id: data.match(/\"interactionId\":(\d+)/)[1],
            sign: sign || null
          })
        } else if (data.match(/配置异常/) && sign) {
          await JDMagicCube(s, sign == 2 ? 1 : null)
          reject()
        } else {
          resolve(null)
        }
      } catch (eor) {
        $nobyda.AnError("京东魔方-查询", "JDCube", eor, response, data)
        reject()
      }
    })
    if (out) setTimeout(reject, out + s)
  }).then(data => {
    return JDMagicCubeSign(s, data)
  }, () => {});
}

function JDMagicCubeSign(s, id) {
  return new Promise(resolve => {
    setTimeout(() => {
      const JDMCUrl = {
        url: `https://api.m.jd.com/client.action?functionId=getNewsInteractionLotteryInfo&appid=smfe${id?`&body=${encodeURIComponent(`{${id.sign?`"sign":${id.sign},`:``}"interactionId":${id.id}}`)}`:``}`,
        headers: {
          Cookie: KEY,
        }
      };
      $nobyda.get(JDMCUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            const cc = JSON.parse(data)
            if (data.match(/(\"name\":)/)) {
              console.log("\n" + "京东商城-魔方签到成功 " + Details)
              merge.JDCube.success = 1
              if (data.match(/(\"name\":\"京豆\")/)) {
                merge.JDCube.bean = cc.result.lotteryInfo.quantity || 0
                merge.JDCube.notify = `京东商城-魔方: 成功, 明细: ${merge.JDCube.bean || `无`}京豆 🐶`
              } else {
                merge.JDCube.notify = `京东商城-魔方: 成功, 明细: ${cc.result.lotteryInfo.name || `未知`} 🎉`
              }
            } else {
              console.log("\n" + "京东商城-魔方签到失败 " + Details)
              merge.JDCube.fail = 1
              if (data.match(/(一闪而过|已签到|已领取)/)) {
                merge.JDCube.notify = "京东商城-魔方: 失败, 原因: 无机会 ⚠️"
              } else if (data.match(/(不存在|已结束)/)) {
                merge.JDCube.notify = "京东商城-魔方: 失败, 原因: 活动已结束 ⚠️"
              } else if (data.match(/(\"code\":3)/)) {
                merge.JDCube.notify = "京东商城-魔方: 失败, 原因: Cookie失效‼️"
              } else {
                merge.JDCube.notify = "京东商城-魔方: 失败, 原因: 未知 ⚠️"
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-魔方", "JDCube", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingDongSpeedUp(s, id) {
  if (!merge.SpeedUp) merge.SpeedUp = {}, merge.SpeedUp.bean = 0;
  return new Promise(resolve => {
    if (disable("SpeedUp")) return resolve()
    setTimeout(() => {
      const GameUrl = {
        url: `https://api.m.jd.com/?appid=memberTaskCenter&functionId=flyTask_${id?`start&body=%7B%22source%22%3A%22game%22%2C%22source_id%22%3A${id}%7D`:`state&body=%7B%22source%22%3A%22game%22%7D`}`,
        headers: {
          Referer: 'https://h5.m.jd.com/babelDiy/Zeus/6yCQo2eDJPbyPXrC3eMCtMWZ9ey/index.html',
          Cookie: KEY
        }
      };
      $nobyda.get(GameUrl, async function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            var cc = JSON.parse(data)
            if (!id) {
              var status = $nobyda.ItemIsUsed ? "再次检查" : merge.SpeedUp.notify ? "查询本次" : "查询上次"
              console.log(`\n天天加速-${status}任务 ${Details}`)
            } else {
              console.log(`\n天天加速-开始${$nobyda.ItemIsUsed?`下轮`:`本次`}任务 ${Details}`)
            }
            if (cc.message == "not login") {
              merge.SpeedUp.fail = 1
              merge.SpeedUp.notify = "京东天天-加速: 失败, 原因: Cookie失效‼️"
              console.log("\n天天加速-Cookie失效")
            } else if (cc.message == "success") {
              if (cc.data.task_status == 0 && cc.data.source_id) {
                if ($nobyda.ItemIsUsed) { //如果使用道具后再次开始任务, 则收到奖励
                  console.log("\n天天加速-领取本次奖励成功")
                  merge.SpeedUp.bean += cc.data.beans_num || 0
                  merge.SpeedUp.success = 1
                  merge.SpeedUp.notify = `京东天天-加速: 成功, 明细: ${merge.SpeedUp.bean || `无`}京豆 🐶`
                }
                await JingDongSpeedUp(s, cc.data.source_id)
              } else if (cc.data.task_status == 1) {
                const percent = Math.round((cc.data.done_distance / cc.data.distance) * 100)
                console.log(`\n天天加速-目前结束时间: \n${cc.data.end_time} [${percent}%]`)
                $nobyda.ItemIsUsed = false
                if (!$nobyda.isAllEvents) await JDSpaceEvent(s); //处理太空事件
                if (!$nobyda.isAlltasks) await JDQueryTask(s); //处理太空任务
                var step3 = await JDQueryTaskID(s); //查询道具ID
                var step4 = await JDUseProps(1000 + s, step3); //使用道具
                if (step4 && $nobyda.ItemIsUsed) { //如果使用了道具, 则再次检查任务
                  await JingDongSpeedUp(s)
                } else {
                  $nobyda.isAllEvents = false; //避免多账号问题
                  $nobyda.isAlltasks = false;
                  $nobyda.tryAgain = false;
                  if (!merge.SpeedUp.notify) {
                    merge.SpeedUp.fail = 1
                    merge.SpeedUp.notify = `京东天天-加速: 失败, 加速中${percent<10?`  `:``}[${percent}%] ⚠️`
                  }
                }
              } else if (cc.data.task_status == 2) {
                merge.SpeedUp.bean = cc.data.beans_num || 0
                merge.SpeedUp.notify = `京东天天-加速: 成功, 明细: ${merge.SpeedUp.bean || `无`}京豆 🐶`
                merge.SpeedUp.success = 1
                console.log("\n天天加速-领取上次奖励成功")
                await JingDongSpeedUp(s, null)
              } else {
                merge.SpeedUp.fail = 1
                merge.SpeedUp.notify = "京东天天-加速: 失败, 原因: 未知 ⚠️"
                console.log("\n" + "天天加速-判断状态码失败")
              }
            } else {
              if (data.match(/领过此任务/)) { //处理任务频繁问题
                if (!$nobyda.tryAgain || $nobyda.tryAgain < 3) { //避免死循环
                  $nobyda.tryAgain ? $nobyda.tryAgain += 1 : $nobyda.tryAgain = 1
                  console.log(`\n天天加速-延迟一秒查询 (${$nobyda.tryAgain})`);
                  await JingDongSpeedUp(1000);
                } else {
                  $nobyda.tryAgain = false;
                  console.log(`\n天天加速-放弃查询任务`);
                }
                return
              }
              if (!merge.SpeedUp.notify) {
                merge.SpeedUp.fail = 1
                merge.SpeedUp.notify = "京东天天-加速: 失败, 原因: 无任务 ⚠️"
              }
              console.log("\n天天加速-判断状态失败")
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东天天-加速", "SpeedUp", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JDSpaceEvent(s) {
  return new Promise(resolve => {
    var spaceEvents = [];
    $nobyda.get({
      url: `https://api.m.jd.com/?appid=memberTaskCenter&functionId=spaceEvent_list&body=%7B%22source%22%3A%22game%22%7D`,
      headers: {
        Referer: 'https://h5.m.jd.com/babelDiy/Zeus/6yCQo2eDJPbyPXrC3eMCtMWZ9ey/index.html',
        Cookie: KEY
      }
    }, (error, response, data) => {
      try {
        if (error) throw new Error(error)
        const cc = JSON.parse(data);
        const Details = LogDetails ? "response:\n" + data : '';
        if (cc.message === "success" && cc.data.length > 0) {
          for (var item of cc.data) {
            if (item.status === 1) {
              for (var j of item.options) {
                if (j.type === 1) {
                  spaceEvents.push({
                    "id": item.id,
                    "value": j.value
                  })
                }
              }
            }
          }
          if (spaceEvents && spaceEvents.length > 0) {
            console.log(`\n天天加速-查询到${spaceEvents.length}个有效事件 ${Details}`)
          } else {
            console.log(`\n天天加速-暂无太空事件 ${Details}`)
          }
        } else {
          console.log(`\n天天加速-太空事件为空 ${Details}`)
        }
      } catch (eor) {
        $nobyda.AnError("太空事件-查询", "SpeedUp", eor, response, data)
      } finally {
        resolve(spaceEvents)
      }
    })
    if (out) setTimeout(resolve, out + s)
  }).then(async (list) => {
    await new Promise(resolve => {
      if (list && list.length > 0) {
        var spaceEventCount = 0;
        var spaceNumTask = 0;
        for (var item of list) {
          $nobyda.get({
            url: `https://api.m.jd.com/?appid=memberTaskCenter&functionId=spaceEvent_handleEvent&body=%7B%22source%22%3A%22game%22%2C%22eventId%22%3A${item.id}%2C%22option%22%3A%22${item.value}%22%7D`,
            headers: {
              Referer: 'https://h5.m.jd.com/babelDiy/Zeus/6yCQo2eDJPbyPXrC3eMCtMWZ9ey/index.html',
              Cookie: KEY
            }
          }, (error, response, data) => {
            try {
              spaceEventCount++
              if (error) throw new Error(error)
              const cc = JSON.parse(data);
              const Details = LogDetails ? "response:\n" + data : '';
              console.log(`\n天天加速-尝试领取第${spaceEventCount}个事件 ${Details}`)
              if (cc.message == "success" && cc.success) {
                spaceNumTask += 1;
              }
            } catch (eor) {
              $nobyda.AnError("太空事件-领取", "SpeedUp", eor, response, data)
            } finally {
              if (list.length == spaceEventCount) {
                if (list.length == spaceNumTask) $nobyda.isAllEvents = true; //避免重复查询
                console.log(`\n天天加速-已成功领取${spaceNumTask}个事件`)
                resolve()
              }
            }
          })
        }
        if (out) setTimeout(resolve, out + s)
      } else {
        $nobyda.isAllEvents = true; //避免重复查询
        resolve()
      }
    })
  })
}

function JDQueryTask(s) {
  return new Promise(resolve => {
    setTimeout(() => {
      var TaskID = "";
      const QueryUrl = {
        url: 'https://api.m.jd.com/?appid=memberTaskCenter&functionId=energyProp_list&body=%7B%22source%22%3A%22game%22%7D',
        headers: {
          Referer: 'https://h5.m.jd.com/babelDiy/Zeus/6yCQo2eDJPbyPXrC3eMCtMWZ9ey/index.html',
          Cookie: KEY
        }
      };
      $nobyda.get(QueryUrl, async function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const cc = JSON.parse(data)
            const Details = LogDetails ? "response:\n" + data : '';
            if (cc.message == "success" && cc.data.length > 0) {
              for (var i = 0; i < cc.data.length; i++) {
                if (cc.data[i].thaw_time == 0) {
                  TaskID += cc.data[i].id + ",";
                }
              }
              if (TaskID.length > 0) {
                TaskID = TaskID.substr(0, TaskID.length - 1).split(",")
                console.log("\n天天加速-查询到" + TaskID.length + "个有效道具 " + Details)
              } else {
                console.log("\n天天加速-暂无有效道具 " + Details)
              }
            } else {
              console.log("\n天天加速-查询无道具 " + Details)
            }
          }
        } catch (eor) {
          $nobyda.AnError("查询道具-加速", "SpeedUp", eor, response, data)
        } finally {
          resolve(TaskID)
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  }).then(async (CID) => {
    await new Promise(resolve => {
      var NumTask = 0
      if (CID) {
        var count = 0
        for (var i = 0; i < CID.length; i++) {
          const TUrl = {
            url: 'https://api.m.jd.com/?appid=memberTaskCenter&functionId=energyProp_gain&body=%7B%22source%22%3A%22game%22%2C%22energy_id%22%3A' + CID[i] + '%7D',
            headers: {
              Referer: 'https://h5.m.jd.com/babelDiy/Zeus/6yCQo2eDJPbyPXrC3eMCtMWZ9ey/index.html',
              Cookie: KEY
            }
          };
          $nobyda.get(TUrl, function(error, response, data) {
            try {
              count++
              if (error) {
                throw new Error(error)
              } else {
                const cc = JSON.parse(data)
                const Details = LogDetails ? "response:\n" + data : '';
                console.log("\n天天加速-尝试领取第" + count + "个道具 " + Details)
                if (cc.message == 'success') {
                  NumTask += 1
                }
              }
            } catch (eor) {
              $nobyda.AnError("领取道具-加速", "SpeedUp", eor, response, data)
            } finally {
              if (CID.length == count) {
                if (CID.length == NumTask) $nobyda.isAlltasks = true; //避免重复查询
                console.log("\n天天加速-已成功领取" + NumTask + "个道具")
                resolve(NumTask)
              }
            }
          })
        }
        if (out) setTimeout(resolve, out + s)
      } else {
        $nobyda.isAlltasks = true; //避免重复查询
        resolve(NumTask)
      }
    })
  })
}

function JDQueryTaskID(s) {
  return new Promise(resolve => {
    var TaskCID = ""
    setTimeout(() => {
      const EUrl = {
        url: 'https://api.m.jd.com/?appid=memberTaskCenter&functionId=energyProp_usalbeList&body=%7B%22source%22%3A%22game%22%7D',
        headers: {
          Referer: 'https://h5.m.jd.com/babelDiy/Zeus/6yCQo2eDJPbyPXrC3eMCtMWZ9ey/index.html',
          Cookie: KEY
        }
      };
      $nobyda.get(EUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const cc = JSON.parse(data)
            const Details = LogDetails ? "response:\n" + data : '';
            if (cc.data.length > 0) {
              for (var i = 0; i < cc.data.length; i++) {
                if (cc.data[i].id) {
                  TaskCID += cc.data[i].id + ",";
                }
              }
              if (TaskCID.length > 0) {
                TaskCID = TaskCID.substr(0, TaskCID.length - 1).split(",")
                console.log(`\n天天加速-查询成功${TaskCID.length}个道具ID ${Details}`)
              } else {
                console.log(`\n天天加速-暂无有效道具ID ${Details}`)
              }
            } else {
              console.log(`\n天天加速-查询无道具ID ${Details}`)
            }
          }
        } catch (eor) {
          $nobyda.AnError("查询号码-加速", "SpeedUp", eor, response, data)
        } finally {
          resolve(TaskCID)
        }
      })
    }, s + 200)
    if (out) setTimeout(resolve, out + s)
  });
}

function JDUseProps(s, PropID) {
  return new Promise(async (resolve) => {
    $nobyda.PropCount = 0
    $nobyda.PropNumTask = 0
    if (!PropID) return resolve()
    for (var i = 0; i < PropID.length; i++) {
      const sep = await new Promise(resolve => {
        $nobyda.get({
          url: `https://api.m.jd.com/?appid=memberTaskCenter&functionId=energyProp_use&body=%7B%22source%22%3A%22game%22%2C%22energy_id%22%3A%22${PropID[i]}%22%7D`,
          headers: {
            Referer: 'https://h5.m.jd.com/babelDiy/Zeus/6yCQo2eDJPbyPXrC3eMCtMWZ9ey/index.html',
            Cookie: KEY
          }
        }, (error, response, data) => {
          try {
            $nobyda.PropCount++
              if (error) {
                throw new Error(error)
              } else {
                const cc = JSON.parse(data)
                const Details = LogDetails ? "response:\n" + data : '';
                console.log(`\n天天加速-尝试使用第${$nobyda.PropCount}个道具 (${s/1000}s)${Details}`)
                if (cc.message == 'success' && cc.success == true) {
                  $nobyda.PropNumTask += 1
                }
              }
          } catch (eor) {
            $nobyda.AnError("使用道具-加速", "SpeedUp", eor, response, data)
          } finally {
            if (PropID.length == $nobyda.PropCount) {
              if ($nobyda.PropNumTask) $nobyda.ItemIsUsed = true;
              console.log(`\n天天加速-已成功使用${$nobyda.PropNumTask}个道具`)
              resolve(true)
            } else {
              setTimeout(resolve, s)
            }
          }
        })
      })
      if (sep) resolve($nobyda.PropNumTask);
    }
  });
}

function JingDongSubsidy(s) {
  merge.subsidy = {};
  return new Promise(resolve => {
    if (disable("subsidy")) return resolve()
    setTimeout(() => {
      const subsidyUrl = {
        url: 'https://ms.jr.jd.com/gw/generic/uc/h5/m/signIn7',
        headers: {
          Referer: "https://active.jd.com/forever/cashback/index",
          Cookie: KEY
        }
      };
      $nobyda.get(subsidyUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            const cc = JSON.parse(data)
            if (cc.resultCode == 0 && cc.resultData.data.thisAmount) {
              console.log("\n" + "京东商城-金贴签到成功 " + Details)
              merge.subsidy.subsidy = cc.resultData.data.thisAmountStr
              merge.subsidy.notify = `京东商城-金贴: 成功, 明细: ${merge.subsidy.subsidy||`无`}金贴 💰`
              merge.subsidy.success = 1
            } else {
              console.log("\n" + "京东商城-金贴签到失败 " + Details)
              merge.subsidy.fail = 1
              if (data.match(/已存在|"thisAmount":0/)) {
                merge.subsidy.notify = "京东商城-金贴: 失败, 原因: 已签过 ⚠️"
              } else if (data.match(/请先登录/)) {
                merge.subsidy.notify = "京东商城-金贴: 失败, 原因: Cookie失效‼️"
              } else {
                merge.subsidy.notify = "京东商城-金贴: 失败, 原因: 未知 ⚠️"
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-金贴", "subsidy", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingRongDoll(s, key, title, code, type, num, award, belong) {
  merge[key] = {};
  return new Promise(resolve => {
    if (disable(key)) return resolve()
    setTimeout(() => {
      const DollUrl = {
        url: "https://nu.jr.jd.com/gw/generic/jrm/h5/m/process",
        headers: {
          Cookie: KEY
        },
        body: `reqData=${encodeURIComponent(`{"actCode":"${code}","type":${type?type:`3`}${code=='F68B2C3E71'?`,"frontParam":{"belong":"${belong}"}`:code==`1DF13833F7`?`,"frontParam":{"channel":"JR","belong":4}`:``}}`)}`
      };
      $nobyda.post(DollUrl, async function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            var cc = JSON.parse(data)
            const Details = LogDetails ? "response:\n" + data : '';
            if (cc.resultCode == 0) {
              if (cc.resultData.data.businessData != null) {
                console.log(`\n${title}查询成功 ${Details}`)
                if (cc.resultData.data.businessData.pickStatus == 2) {
                  if (data.match(/\"rewardPrice\":\"\d.*?\"/)) {
                    const JRDoll_bean = data.match(/\"rewardPrice\":\"(\d.*?)\"/)[1]
                    const JRDoll_type = data.match(/\"rewardName\":\"金贴奖励\"/) ? true : false
                    await JingRongDoll(s, key, title, code, '4', JRDoll_bean, JRDoll_type)
                  } else {
                    merge[key].success = 1
                    merge[key].notify = `${title}: 成功, 明细: 无奖励 🐶`
                  }
                } else if (code == 'F68B2C3E71' || code == '1DF13833F7') {
                  if (!data.match(/"businessCode":"30\dss?q"/)) {
                    merge[key].success = 1
                    const ct = data.match(/\"count\":\"?(\d.*?)\"?,/)
                    if (code == 'F68B2C3E71' && belong == 'xianjin') {
                      merge[key].Money = ct ? ct[1] > 9 ? `0.${ct[1]}` : `0.0${ct[1]}` : 0
                      merge[key].notify = `${title}: 成功, 明细: ${merge[key].Money||`无`}现金 💰`
                    } else if (code == 'F68B2C3E71' && belong == 'jingdou') {
                      merge[key].bean = ct ? ct[1] : 0;
                      merge[key].notify = `${title}: 成功, 明细: ${merge[key].bean||`无`}京豆 🐶`
                    } else if (code == '1DF13833F7') {
                      merge[key].subsidy = ct ? ct[1] : 0;
                      merge[key].notify = `${title}: 成功, 明细: ${merge[key].subsidy||`无`}金贴 💰`
                    }
                  } else {
                    const es = cc.resultData.data.businessMsg
                    const ep = cc.resultData.data.businessData.businessMsg
                    const tp = data.match(/已领取|300ss?q/) ? `已签过` : `${ep||es||cc.resultMsg||`未知`}`
                    merge[key].notify = `${title}: 失败, 原因: ${tp} ⚠️`
                    merge[key].fail = 1
                  }
                } else {
                  merge[key].notify = `${title}: 失败, 原因: 已签过 ⚠️`;
                  merge[key].fail = 1
                }
              } else if (cc.resultData.data.businessCode == 200) {
                console.log(`\n${title}签到成功 ${Details}`)
                if (!award) {
                  merge[key].bean = num ? num.match(/\d+/)[0] : 0
                } else {
                  merge[key].subsidy = num || 0
                }
                merge[key].success = 1
                merge[key].notify = `${title}: 成功, 明细: ${(award?num:merge[key].bean)||`无`}${award?`金贴 💰`:`京豆 🐶`}`
              } else {
                console.log(`\n${title}领取异常 ${Details}`)
                if (num) console.log(`\n${title} 请尝试手动领取, 预计可得${num}${award?`金贴`:`京豆`}: \nhttps://uf1.jr.jd.com/up/redEnvelopes/index.html?actCode=${code}\n`);
                merge[key].fail = 1;
                merge[key].notify = `${title}: 失败, 原因: 领取异常 ⚠️`;
              }
            } else {
              console.log(`\n${title}签到失败 ${Details}`)
              const redata = typeof(cc.resultData) == 'string' ? cc.resultData : ''
              merge[key].notify = `${title}: 失败, ${cc.resultCode==3?`原因: Cookie失效‼️`:`${redata||'原因: 未知 ⚠️'}`}`
              merge[key].fail = 1;
            }
          }
        } catch (eor) {
          $nobyda.AnError(title, key, eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JDOverseas(s) {
  merge.Overseas = {};
  return new Promise(resolve => {
    if (disable("Overseas")) return resolve()
    setTimeout(() => {
      const OverseasUrl = {
        url: 'https://api.m.jd.com/client.action?functionId=checkin',
        headers: {
          Cookie: KEY
        },
        body: "body=%7B%7D&build=167237&client=apple&clientVersion=9.0.0&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&partner=apple&scope=11&sign=e27f8b904040a0e3c99b87fc27e09c87&st=1591730990449&sv=101"
      };
      $nobyda.post(OverseasUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const Details = LogDetails ? "response:\n" + data : '';
            if (data.match(/\"type\":\d+?,/)) {
              console.log("\n" + "京东商城-国际签到成功 " + Details)
              merge.Overseas.success = 1
              if (data.match(/\"jdBeanAmount\":[1-9]+/)) {
                merge.Overseas.bean = data.match(/\"jdBeanAmount\":(\d+)/)[1]
                merge.Overseas.notify = "京东商城-国际: 成功, 明细: " + merge.Overseas.bean + "京豆 🐶"
              } else {
                merge.Overseas.notify = "京东商城-国际: 成功, 明细: 无京豆 🐶"
              }
            } else {
              console.log("\n" + "京东商城-国际签到失败 " + Details)
              merge.Overseas.fail = 1
              if (data.match(/(\"code\":\"13\"|重复签到)/)) {
                merge.Overseas.notify = "京东商城-国际: 失败, 原因: 已签过 ⚠️"
              } else if (data.match(/\"code\":\"(-1|3)\"/)) {
                merge.Overseas.notify = "京东商城-国际: 失败, 原因: Cookie失效‼️"
              } else {
                merge.Overseas.notify = "京东商城-国际: 失败, 原因: 未知 ⚠️"
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-国际", "Overseas", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingDongGetCash(s) {
  merge.JDGetCash = {};
  return new Promise(resolve => {
    if (disable("JDGetCash")) return resolve()
    setTimeout(() => {
      const GetCashUrl = {
        url: 'https://api.m.jd.com/client.action?functionId=cash_sign&body=%7B%22remind%22%3A0%2C%22inviteCode%22%3A%22%22%2C%22type%22%3A0%2C%22breakReward%22%3A0%7D&client=apple&clientVersion=9.0.8&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&sign=7e2f8bcec13978a691567257af4fdce9&st=1596954745073&sv=111',
        headers: {
          Cookie: KEY,
        }
      };
      $nobyda.get(GetCashUrl, function(error, response, data) {
        try {
          if (error) {
            throw new Error(error)
          } else {
            const cc = JSON.parse(data);
            const Details = LogDetails ? "response:\n" + data : '';
            if (cc.data.success && cc.data.result) {
              console.log("\n" + "京东商城-现金签到成功 " + Details)
              merge.JDGetCash.success = 1
              merge.JDGetCash.Money = cc.data.result.signCash || 0
              merge.JDGetCash.notify = `京东商城-现金: 成功, 明细: ${cc.data.result.signCash||`无`}现金 💰`
            } else {
              console.log("\n" + "京东商城-现金签到失败 " + Details)
              merge.JDGetCash.fail = 1
              if (data.match(/\"bizCode\":201|已经签过/)) {
                merge.JDGetCash.notify = "京东商城-现金: 失败, 原因: 已签过 ⚠️"
              } else if (data.match(/\"code\":300|退出登录/)) {
                merge.JDGetCash.notify = "京东商城-现金: 失败, 原因: Cookie失效‼️"
              } else {
                merge.JDGetCash.notify = "京东商城-现金: 失败, 原因: 未知 ⚠️"
              }
            }
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-现金", "JDGetCash", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingDongWebcasts(s) {
  merge.JDWebcasts = {};
  return new Promise(resolve => {
    if (disable("JDWebcasts")) return resolve()
    setTimeout(() => {
      $nobyda.get({
        url: `https://api.m.jd.com/api?functionId=getChannelTaskRewardToM&appid=h5-live&body=%7B%22type%22%3A%22signTask%22%2C%22itemId%22%3A%221%22%7D`,
        headers: {
          Cookie: KEY,
          Origin: `https://h.m.jd.com`
        }
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error)
          const cc = JSON.parse(data);
          const Details = LogDetails ? "response:\n" + data : '';
          if (cc.code == 0 && cc.subCode == 0) {
            console.log(`\n京东商城-直播签到成功 ${Details}`)
            merge.JDWebcasts.bean = cc.sum || 0
            merge.JDWebcasts.success = 1
            merge.JDWebcasts.notify = `京东商城-直播: 成功, 明细: ${merge.JDWebcasts.bean||`无`}京豆 🐶`
          } else {
            console.log(`\n京东商城-直播签到失败 ${Details}`)
            const tp = data.match(/擦肩而过/) ? `无机会` : cc.code == 3 ? `Cookie失效` : `${cc.msg||`未知`}`
            merge.JDWebcasts.notify = `京东商城-直播: 失败, 原因: ${tp}${cc.code==3?`‼️`:` ⚠️`}`
            merge.JDWebcasts.fail = 1
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-直播", "JDWebcasts", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JDTakeaLook(s) {
  merge.TakeaLook = {};
  return new Promise(resolve => {
    if (disable("TakeaLook")) return resolve()
    setTimeout(() => {
      $nobyda.get({
        url: 'https://api.m.jd.com/client.action?functionId=discTaskList&body=%7B%22bizType%22%3A1%2C%22referPageId%22%3A%22discRecommend%22%7D&client=apple&clientVersion=9.1.6&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&sign=17061147fe8e0eb10edfe8d9968b6d66&st=1601138337675&sv=102',
        headers: {
          Cookie: KEY
        }
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error);
          const cc = JSON.parse(data);
          const Details = LogDetails ? "response:\n" + data : '';
          const tm = parseInt((Date.now() + 28800000) / 86400000) * 86400000 - 28800000
          if (data.match(/签到成功/) && !data.match(tm)) {
            console.log(`\n京东发现-看看签到成功 ${Details}`)
            const aw = data.match(/\"签到成功，获得(\d+)京豆\"/)
            merge.TakeaLook.success = 1
            merge.TakeaLook.bean = aw ? aw[1] : 0
            merge.TakeaLook.notify = `京东发现-看看: 成功, 明细: ${merge.TakeaLook.bean||`无`}京豆 🐶`
          } else {
            console.log(`\n京东发现-看看签到失败 ${Details}`)
            const tp = data.match(tm) ? `已签过` : cc.busiCode == 2001 ? `Cookie失效` : `${cc.message||`未知`}`
            merge.TakeaLook.notify = `京东发现-看看: 失败, 原因: ${tp}${cc.busiCode==2001?`‼️`:` ⚠️`}`
            merge.TakeaLook.fail = 1
          }
        } catch (eor) {
          $nobyda.AnError("京东发现-看看", "TakeaLook", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JingDongStore(s) {
  merge.JDGStore = {};
  return new Promise(resolve => {
    if (disable("JDGStore")) return resolve()
    setTimeout(() => {
      $nobyda.get({
        url: 'https://api.m.jd.com/api?appid=jdsupermarket&functionId=smtg_sign&clientVersion=8.0.0&client=m&body=%7B%7D',
        headers: {
          Cookie: KEY,
          Origin: `https://jdsupermarket.jd.com`
        }
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error);
          const cc = JSON.parse(data);
          const Details = LogDetails ? "response:\n" + data : '';
          if (cc.data && cc.data.success === true && cc.data.bizCode === 0) {
            console.log(`\n京东商城-超市签到成功 ${Details}`)
            merge.JDGStore.success = 1
            merge.JDGStore.bean = cc.data.result.jdBeanCount || 0
            merge.JDGStore.notify = `京东商城-超市: 成功, 明细: ${merge.JDGStore.bean||`无`}京豆 🐶`
          } else {
            if (!cc.data) cc.data = {}
            console.log(`\n京东商城-超市签到失败 ${Details}`)
            const tp = cc.data.bizCode == 811 ? `已签过` : cc.data.bizCode == 300 ? `Cookie失效` : `${cc.data.bizMsg||`未知`}`
            merge.JDGStore.notify = `京东商城-超市: 失败, 原因: ${tp}${cc.data.bizCode==300?`‼️`:` ⚠️`}`
            merge.JDGStore.fail = 1
          }
        } catch (eor) {
          $nobyda.AnError("京东商城-超市", "JDGStore", eor, response, data)
        } finally {
          resolve()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  });
}

function JDSecKilling(s) {
  merge.JDSecKill = {};
  return new Promise((resolve, reject) => {
    if (disable("JDSecKill")) return reject();
    setTimeout(() => {
      $nobyda.post({
        url: 'https://api.m.jd.com/client.action',
        headers: {
          Cookie: KEY,
          Origin: 'https://h5.m.jd.com'
        },
        body: 'functionId=freshManHomePage&body=%7B%7D&client=wh5&appid=SecKill2020'
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error);
          const Details = LogDetails ? "response:\n" + data : '';
          const cc = JSON.parse(data);
          if (cc.code == 203 || cc.code == 3 || cc.code == 101) {
            merge.JDSecKill.notify = `京东秒杀-红包: 失败, 原因: Cookie失效‼️`;
          } else if (cc.result && cc.result.projectId && cc.result.taskId) {
            console.log(`\n京东秒杀-红包查询成功 ${Details}`)
            return resolve({
              projectId: cc.result.projectId,
              taskId: cc.result.taskId
            })
          } else {
            merge.JDSecKill.notify = `京东秒杀-红包: 失败, 暂无有效活动 ⚠️`;
          }
          merge.JDSecKill.fail = 1;
          console.log(`\n京东秒杀-红包查询失败 ${Details}`)
          reject()
        } catch (eor) {
          $nobyda.AnError("京东秒杀-查询", "JDSecKill", eor, response, data)
          reject()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  }).then(async (id) => {
    await new Promise(resolve => {
      $nobyda.post({
        url: 'https://api.m.jd.com/client.action',
        headers: {
          Cookie: KEY,
          Origin: 'https://h5.m.jd.com'
        },
        body: `functionId=doInteractiveAssignment&body=%7B%22encryptProjectId%22%3A%22${id.projectId}%22%2C%22encryptAssignmentId%22%3A%22${id.taskId}%22%2C%22completionFlag%22%3Atrue%7D&client=wh5&appid=SecKill2020`
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error);
          const Details = LogDetails ? "response:\n" + data : '';
          const cc = JSON.parse(data);
          if (cc.msg == 'success' && cc.subCode == 0) {
            console.log(`\n京东秒杀-红包签到成功 ${Details}`);
            const qt = data.match(/"discount":(\d.*?),/);
            merge.JDSecKill.success = 1;
            merge.JDSecKill.Cash = qt ? qt[1] : 0;
            merge.JDSecKill.notify = `京东秒杀-红包: 成功, 明细: ${merge.JDSecKill.Cash||`无`}红包 🧧`;
          } else {
            console.log(`\n京东秒杀-红包签到失败 ${Details}`);
            merge.JDSecKill.fail = 1;
            merge.JDSecKill.notify = `京东秒杀-红包: 失败, ${cc.subCode==103?`原因: 已领取`:cc.msg?cc.msg:`原因: 未知`} ⚠️`;
          }
        } catch (eor) {
          $nobyda.AnError("京东秒杀-领取", "JDSecKill", eor, response, data);
        } finally {
          resolve();
        }
      })
    })
  }, () => {});
}

function JRLuckyLottery(s) { //https://jdda.jd.com/app/hd/#/turntable
  merge.JRLottery = {};
  return new Promise(resolve => {
    if (disable("JRLottery")) return resolve()
    setTimeout(() => {
      $nobyda.post({
        url: 'https://ms.jr.jd.com/gw/generic/syh_yxmx/h5/m/handleSign',
        headers: {
          Cookie: KEY
        },
        body: `reqData=${encodeURIComponent(`{"activityNo":"e2d1b240d5674def8178be6b4faac5b6","signType":"1","encryptSign":""}`)}`
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error);
          const cc = JSON.parse(data);
          const Details = LogDetails ? "response:\n" + data : '';
          if (cc.resultCode == 0 && cc.resultData && cc.resultData.resultCode == 0) {
            console.log(`\n京东金融-抽奖签到成功 ${Details}`);
            merge.JRLottery.success = 1;
            merge.JRLottery.bean = cc.resultData.data.rewardList[0].rewardCount || 0
            merge.JRLottery.notify = `京东金融-抽奖: 成功, 明细: ${merge.JRLottery.bean||`无`}京豆 🐶`;
          } else {
            console.log(`\n京东金融-抽奖签到失败 ${Details}`);
            const tp = data.match(/重复签到/) ? `已签过` : cc.resultCode == 3 ? `Cookie失效` : `${cc.resultMsg||`未知`}`;
            merge.JRLottery.notify = `京东金融-抽奖: 失败, 原因: ${tp}${cc.resultCode==3?`‼️`:` ⚠️`}`;
            merge.JRLottery.fail = 1;
          }
        } catch (eor) {
          $nobyda.AnError("京东金融-抽奖", "JRLottery", eor, response, data);
        } finally {
          resolve();
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s);
  });
}

function JingDongBuyCar(s, ActId) {
  merge.JDBuyCar = {};
  return new Promise((resolve, reject) => {
    if (disable("JDBuyCar")) return reject();
    setTimeout(() => {
      $nobyda.get({
        url: 'https://cgame-stadium.jd.com/api/v1/first/login',
        headers: {
          Cookie: KEY,
          ActivityId: ActId
        }
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error);
          const Details = LogDetails ? "response:\n" + data : '';
          console.log(`\n京东汽车-检查签到状态 ${Details}`)
          const cc = JSON.parse(data);
          if (cc.status && cc.data && cc.data.firstLoginStatus) {
            resolve()
          } else {
            const qt = cc.status && cc.data && cc.data.firstLoginStatus === false ? `已签过` : cc.error && cc.error.code == 2000 ? `Cookie失效` : cc.error && cc.error.msg ? cc.error.msg : `未知`
            merge.JDBuyCar.notify = `京东商城-汽车: 失败, 原因: ${qt}${cc.error&&cc.error.code==2000?`‼️`:` ⚠️`}`
            merge.JDBuyCar.fail = 1;
            reject()
          }
        } catch (eor) {
          $nobyda.AnError("京东汽车-状态", "JDBuyCar", eor, response, data)
          reject()
        }
      })
    }, s)
    if (out) setTimeout(resolve, out + s)
  }).then(async () => {
    await new Promise(resolve => {
      $nobyda.post({
        url: 'https://cgame-stadium.jd.com/api/v1/sign',
        headers: {
          Cookie: KEY,
          ActivityId: ActId
        }
      }, (error, response, data) => {
        try {
          if (error) throw new Error(error);
          const Details = LogDetails ? "response:\n" + data : '';
          const cc = JSON.parse(data);
          if (cc.status === true) {
            console.log(`\n京东商城-汽车签到成功 ${Details}`);
            merge.JDBuyCar.success = 1;
            merge.JDBuyCar.bean = cc.data && cc.data.beanNum ? cc.data.beanNum : 0
            merge.JDBuyCar.notify = `京东商城-汽车: 成功, 明细: ${merge.JDBuyCar.bean||`无`}京豆 🐶`;
          } else {
            console.log(`\n京东商城-汽车签到失败 ${Details}`);
            merge.JDBuyCar.fail = 1;
            merge.JDBuyCar.notify = `京东商城-汽车: 失败, ${cc.error&&cc.error.msg?cc.error.msg:`原因: 未知`} ⚠️`;
          }
        } catch (eor) {
          $nobyda.AnError("京东汽车-签到", "JDBuyCar", eor, response, data);
        } finally {
          resolve();
        }
      })
    })
  }, () => {});
}

function TotalSteel() {
  merge.TotalSteel = {};
  return new Promise(resolve => {
    if (disable("TSteel")) return resolve()
    $nobyda.get({
      url: 'https://coin.jd.com/m/gb/getBaseInfo.html',
      headers: {
        Cookie: KEY
      }
    }, (error, response, data) => {
      try {
        if (error) throw new Error(error);
        const Details = LogDetails ? "response:\n" + data : '';
        if (data.match(/(\"gbBalance\":\d+)/)) {
          console.log("\n" + "京东-总钢镚查询成功 " + Details)
          const cc = JSON.parse(data)
          merge.TotalSteel.TSteel = cc.gbBalance
        } else {
          console.log("\n" + "京东-总钢镚查询失败 " + Details)
        }
      } catch (eor) {
        $nobyda.AnError("账户钢镚-查询", "TotalSteel", eor, response, data)
      } finally {
        resolve()
      }
    })
    if (out) setTimeout(resolve, out)
  });
}

function TotalBean() {
  merge.TotalBean = {};
  return new Promise(resolve => {
    if (disable("Qbear")) return resolve()
    $nobyda.post({
      url: 'https://wq.jd.com/user/info/QueryJDUserInfo?sceneval=2',
      headers: {
        Cookie: KEY,
        Referer: "https://wqs.jd.com/my/jingdou/my.shtml?sceneval=2"
      }
    }, (error, response, data) => {
      try {
        if (error) throw new Error(error);
        const Details = LogDetails ? "response:\n" + data : '';
        const cc = JSON.parse(data)
        $nobyda.headUrl = cc.base.headImageUrl
        if (cc.base.jdNum != 0) {
          console.log("\n" + "京东-总京豆查询成功 " + Details)
          merge.TotalBean.Qbear = cc.base.jdNum
        } else {
          console.log("\n" + "京东-总京豆查询失败 " + Details)
        }
        if (data.match(/\"nickname\" ?: ?\"(.+?)\",/)) {
          merge.TotalBean.nickname = cc.base.nickname
        } else if (data.match(/\"no ?login\.?\"/)) {
          merge.TotalBean.nickname = "Cookie失效 ‼️"
        } else {
          merge.TotalBean.nickname = '';
        }
      } catch (eor) {
        $nobyda.AnError("账户京豆-查询", "TotalBean", eor, response, data)
      } finally {
        resolve()
      }
    })
    if (out) setTimeout(resolve, out)
  });
}

function TotalCash() {
  merge.TotalCash = {};
  return new Promise(resolve => {
    if (disable("TCash")) return resolve()
    $nobyda.post({
      url: 'https://api.m.jd.com/client.action?functionId=myhongbao_balance',
      headers: {
        Cookie: KEY
      },
      body: "body=%7B%22fp%22%3A%22-1%22%2C%22appToken%22%3A%22apphongbao_token%22%2C%22childActivityUrl%22%3A%22-1%22%2C%22country%22%3A%22cn%22%2C%22openId%22%3A%22-1%22%2C%22childActivityId%22%3A%22-1%22%2C%22applicantErp%22%3A%22-1%22%2C%22platformId%22%3A%22appHongBao%22%2C%22isRvc%22%3A%22-1%22%2C%22orgType%22%3A%222%22%2C%22activityType%22%3A%221%22%2C%22shshshfpb%22%3A%22-1%22%2C%22platformToken%22%3A%22apphongbao_token%22%2C%22organization%22%3A%22JD%22%2C%22pageClickKey%22%3A%22-1%22%2C%22platform%22%3A%221%22%2C%22eid%22%3A%22-1%22%2C%22appId%22%3A%22appHongBao%22%2C%22childActiveName%22%3A%22-1%22%2C%22shshshfp%22%3A%22-1%22%2C%22jda%22%3A%22-1%22%2C%22extend%22%3A%22-1%22%2C%22shshshfpa%22%3A%22-1%22%2C%22activityArea%22%3A%22-1%22%2C%22childActivityTime%22%3A%22-1%22%7D&client=apple&clientVersion=8.5.0&d_brand=apple&networklibtype=JDNetworkBaseAF&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&sign=fdc04c3ab0ee9148f947d24fb087b55d&st=1581245397648&sv=120"
    }, (error, response, data) => {
      try {
        if (error) throw new Error(error);
        const Details = LogDetails ? "response:\n" + data : '';
        if (data.match(/(\"totalBalance\":\d+)/)) {
          console.log("\n" + "京东-总红包查询成功 " + Details)
          const cc = JSON.parse(data)
          merge.TotalCash.TCash = cc.totalBalance
        } else {
          console.log("\n" + "京东-总红包查询失败 " + Details)
        }
      } catch (eor) {
        $nobyda.AnError("账户红包-查询", "TotalCash", eor, response, data)
      } finally {
        resolve()
      }
    })
    if (out) setTimeout(resolve, out)
  });
}

function TotalSubsidy() {
  merge.TotalSubsidy = {};
  return new Promise(resolve => {
    if (disable("TotalSubsidy")) return resolve()
    $nobyda.get({
      url: 'https://ms.jr.jd.com/gw/generic/uc/h5/m/mySubsidyBalance',
      headers: {
        Cookie: KEY,
        Referer: 'https://active.jd.com/forever/cashback/index?channellv=wojingqb'
      }
    }, (error, response, data) => {
      try {
        if (error) throw new Error(error);
        const cc = JSON.parse(data)
        const Details = LogDetails ? "response:\n" + data : '';
        if (cc.resultCode == 0 && cc.resultData && cc.resultData.data) {
          console.log("\n京东-总金贴查询成功 " + Details)
          merge.TotalSubsidy.TSubsidy = cc.resultData.data.balance || 0
        } else {
          console.log("\n京东-总金贴查询失败 " + Details)
        }
      } catch (eor) {
        $nobyda.AnError("账户金贴-查询", "TotalSubsidy", eor, response, data)
      } finally {
        resolve()
      }
    })
    if (out) setTimeout(resolve, out)
  });
}

function TotalMoney() {
  merge.TotalMoney = {};
  return new Promise(resolve => {
    if (disable("TotalMoney")) return resolve()
    $nobyda.get({
      url: 'https://api.m.jd.com/client.action?functionId=cash_exchangePage&body=%7B%7D&build=167398&client=apple&clientVersion=9.1.9&openudid=1fce88cd05c42fe2b054e846f11bdf33f016d676&sign=762a8e894dea8cbfd91cce4dd5714bc5&st=1602179446935&sv=102',
      headers: {
        Cookie: KEY
      }
    }, (error, response, data) => {
      try {
        if (error) throw new Error(error);
        const cc = JSON.parse(data)
        const Details = LogDetails ? "response:\n" + data : '';
        if (cc.code == 0 && cc.data && cc.data.bizCode == 0 && cc.data.result) {
          console.log("\n京东-总现金查询成功 " + Details)
          merge.TotalMoney.TMoney = cc.data.result.totalMoney || 0
        } else {
          console.log("\n京东-总现金查询失败 " + Details)
        }
      } catch (eor) {
        $nobyda.AnError("账户现金-查询", "TotalMoney", eor, response, data)
      } finally {
        resolve()
      }
    })
    if (out) setTimeout(resolve, out)
  });
}

function disable(Val, name, way) {
  const read = $nobyda.read("JD_DailyBonusDisables")
  const annal = $nobyda.read("JD_Crash_" + Val)
  if (annal && way == 1 && boxdis) {
    var Crash = $nobyda.write("", "JD_Crash_" + Val)
    if (read) {
      if (read.indexOf(Val) == -1) {
        var Crash = $nobyda.write(`${read},${Val}`, "JD_DailyBonusDisables")
        console.log(`\n${name}-触发自动禁用 ‼️`)
        merge[Val].notify = `${name}: 崩溃, 触发自动禁用 ‼️`
        merge[Val].error = 1
        $nobyda.disable = 1
      }
    } else {
      var Crash = $nobyda.write(Val, "JD_DailyBonusDisables")
      console.log(`\n${name}-触发自动禁用 ‼️`)
      merge[Val].notify = `${name}: 崩溃, 触发自动禁用 ‼️`
      merge[Val].error = 1
      $nobyda.disable = 1
    }
    return true
  } else if (way == 1 && boxdis) {
    var Crash = $nobyda.write(name, "JD_Crash_" + Val)
  } else if (way == 2 && annal) {
    var Crash = $nobyda.write("", "JD_Crash_" + Val)
  }
  if (read && read.indexOf(Val) != -1) {
    return true
  } else {
    return false
  }
}

function GetCookie() {
  try {
    if ($request.headers && $request.url.match(/api\.m\.jd\.com.*=signBean/)) {
      var CV = $request.headers['Cookie']
      if (CV.match(/pt_key=.+?;/) && CV.match(/pt_pin=.+?;/)) {
        var CookieValue = CV.match(/pt_key=.+?;/)[0] + CV.match(/pt_pin=.+?;/)[0]
        var CK1 = $nobyda.read("CookieJD")
        var CK2 = $nobyda.read("CookieJD2")
        var AccountOne = CK1 ? CK1.match(/pt_pin=.+?;/) ? CK1.match(/pt_pin=(.+?);/)[1] : null : null
        var AccountTwo = CK2 ? CK2.match(/pt_pin=.+?;/) ? CK2.match(/pt_pin=(.+?);/)[1] : null : null
        var UserName = CookieValue.match(/pt_pin=(.+?);/)[1]
        var DecodeName = decodeURIComponent(UserName)
        if (!AccountOne || UserName == AccountOne) {
          var CookieName = " [账号一] ";
          var CookieKey = "CookieJD";
        } else if (!AccountTwo || UserName == AccountTwo) {
          var CookieName = " [账号二] ";
          var CookieKey = "CookieJD2";
        } else {
          $nobyda.notify("更新京东Cookie失败", "非历史写入账号 ‼️", '请开启脚本内"DeleteCookie"以清空Cookie ‼️')
          return
        }
      } else {
        $nobyda.notify("写入京东Cookie失败", "", "请查看脚本内说明, 登录网页获取 ‼️")
        return
      }
      const RA = $nobyda.read(CookieKey);
      if (RA == CookieValue) {
        console.log(`\n用户名: ${DecodeName}\n与历史京东${CookieName}Cookie相同, 跳过写入 ⚠️`)
      } else {
        const WT = $nobyda.write(CookieValue, CookieKey);
        $nobyda.notify(`用户名: ${DecodeName}`, ``, `${RA?`更新`:`写入`}京东${CookieName}Cookie${WT?`成功 🎉`:`失败 ‼️`}`)
      }
    } else if ($request.url === 'http://www.apple.com/') {
      $nobyda.notify("京东签到", "", "类型错误, 手动运行请选择上下文环境为Cron ⚠️");
    } else {
      $nobyda.notify("京东签到", "写入Cookie失败", "请检查匹配URL或配置内脚本类型 ⚠️");
    }
  } catch (eor) {
    $nobyda.write("", "CookieJD")
    $nobyda.write("", "CookieJD2")
    $nobyda.notify("写入京东Cookie失败", "", '已尝试清空历史Cookie, 请重试 ⚠️')
    console.log(`\n写入京东Cookie出现错误 ‼️\n${JSON.stringify(eor)}\n\n${eor}\n\n${JSON.stringify($request.headers)}\n`)
  } finally {
    $nobyda.done()
  }
}
// Modified from yichahucha
function nobyda() {
  const start = Date.now()
  const isRequest = typeof $request != "undefined"
  const isSurge = typeof $httpClient != "undefined"
  const isQuanX = typeof $task != "undefined"
  const isLoon = typeof $loon != "undefined"
  const isJSBox = typeof $app != "undefined" && typeof $http != "undefined"
  const isNode = typeof require == "function" && !isJSBox;
  const NodeSet = 'CookieSet.json'
  const node = (() => {
    if (isNode) {
      const request = require('request');
      const fs = require("fs");
      return ({
        request,
        fs
      })
    } else {
      return (null)
    }
  })()
  const notify = (title, subtitle, message, rawopts) => {
    const Opts = (rawopts) => { //Modified from https://github.com/chavyleung/scripts/blob/master/Env.js
      if (!rawopts) return rawopts
      if (typeof rawopts === 'string') {
        if (isLoon) return rawopts
        else if (isQuanX) return {
          'open-url': rawopts
        }
        else if (isSurge) return {
          url: rawopts
        }
        else return undefined
      } else if (typeof rawopts === 'object') {
        if (isLoon) {
          let openUrl = rawopts.openUrl || rawopts.url || rawopts['open-url']
          let mediaUrl = rawopts.mediaUrl || rawopts['media-url']
          return {
            openUrl,
            mediaUrl
          }
        } else if (isQuanX) {
          let openUrl = rawopts['open-url'] || rawopts.url || rawopts.openUrl
          let mediaUrl = rawopts['media-url'] || rawopts.mediaUrl
          return {
            'open-url': openUrl,
            'media-url': mediaUrl
          }
        } else if (isSurge) {
          let openUrl = rawopts.url || rawopts.openUrl || rawopts['open-url']
          return {
            url: openUrl
          }
        }
      } else {
        return undefined
      }
    }
    console.log(`${title}\n${subtitle}\n${message}`)
    if (isQuanX) $notify(title, subtitle, message, Opts(rawopts))
    if (isSurge) $notification.post(title, subtitle, message, Opts(rawopts))
    if (isJSBox) $push.schedule({
      title: title,
      body: subtitle ? subtitle + "\n" + message : message
    })
  }
  const write = (value, key) => {
    if (isQuanX) return $prefs.setValueForKey(value, key)
    if (isSurge) return $persistentStore.write(value, key)
    if (isNode) {
      try {
        if (!node.fs.existsSync(NodeSet)) node.fs.writeFileSync(NodeSet, JSON.stringify({}));
        const dataValue = JSON.parse(node.fs.readFileSync(NodeSet));
        if (value) dataValue[key] = value;
        if (!value) delete dataValue[key];
        return node.fs.writeFileSync(NodeSet, JSON.stringify(dataValue));
      } catch (er) {
        return AnError('Node.js持久化写入', null, er);
      }
    }
    if (isJSBox) {
      if (!value) return $file.delete(`shared://${key}.txt`);
      return $file.write({
        data: $data({
          string: value
        }),
        path: `shared://${key}.txt`
      })
    }
  }
  const read = (key) => {
    if (isQuanX) return $prefs.valueForKey(key)
    if (isSurge) return $persistentStore.read(key)
    if (isNode) {
      try {
        if (!node.fs.existsSync(NodeSet)) return null;
        const dataValue = JSON.parse(node.fs.readFileSync(NodeSet))
        return dataValue[key]
      } catch (er) {
        return AnError('Node.js持久化读取', null, er)
      }
    }
    if (isJSBox) {
      if (!$file.exists(`shared://${key}.txt`)) return null;
      return $file.read(`shared://${key}.txt`).string
    }
  }
  const adapterStatus = (response) => {
    if (response) {
      if (response.status) {
        response["statusCode"] = response.status
      } else if (response.statusCode) {
        response["status"] = response.statusCode
      }
    }
    return response
  }
  const get = (options, callback) => {
    options.headers['User-Agent'] = 'JD4iPhone/167169 (iPhone; iOS 13.4.1; Scale/3.00)'
    if (isQuanX) {
      if (typeof options == "string") options = {
        url: options
      }
      options["method"] = "GET"
      //options["opts"] = {
      //  "hints": false
      //}
      $task.fetch(options).then(response => {
        callback(null, adapterStatus(response), response.body)
      }, reason => callback(reason.error, null, null))
    }
    if (isSurge) {
      options.headers['X-Surge-Skip-Scripting'] = false
      $httpClient.get(options, (error, response, body) => {
        callback(error, adapterStatus(response), body)
      })
    }
    if (isNode) {
      node.request(options, (error, response, body) => {
        callback(error, adapterStatus(response), body)
      })
    }
    if (isJSBox) {
      if (typeof options == "string") options = {
        url: options
      }
      options["header"] = options["headers"]
      options["handler"] = function(resp) {
        let error = resp.error;
        if (error) error = JSON.stringify(resp.error)
        let body = resp.data;
        if (typeof body == "object") body = JSON.stringify(resp.data);
        callback(error, adapterStatus(resp.response), body)
      };
      $http.get(options);
    }
  }
  const post = (options, callback) => {
    options.headers['User-Agent'] = 'JD4iPhone/167169 (iPhone; iOS 13.4.1; Scale/3.00)'
    if (options.body) options.headers['Content-Type'] = 'application/x-www-form-urlencoded'
    if (isQuanX) {
      if (typeof options == "string") options = {
        url: options
      }
      options["method"] = "POST"
      //options["opts"] = {
      //  "hints": false
      //}
      $task.fetch(options).then(response => {
        callback(null, adapterStatus(response), response.body)
      }, reason => callback(reason.error, null, null))
    }
    if (isSurge) {
      options.headers['X-Surge-Skip-Scripting'] = false
      $httpClient.post(options, (error, response, body) => {
        callback(error, adapterStatus(response), body)
      })
    }
    if (isNode) {
      node.request.post(options, (error, response, body) => {
        callback(error, adapterStatus(response), body)
      })
    }
    if (isJSBox) {
      if (typeof options == "string") options = {
        url: options
      }
      options["header"] = options["headers"]
      options["handler"] = function(resp) {
        let error = resp.error;
        if (error) error = JSON.stringify(resp.error)
        let body = resp.data;
        if (typeof body == "object") body = JSON.stringify(resp.data)
        callback(error, adapterStatus(resp.response), body)
      }
      $http.post(options);
    }
  }
  const AnError = (name, keyname, er, resp, body) => {
    if (typeof(merge) != "undefined" && keyname) {
      if (!merge[keyname].notify) {
        merge[keyname].notify = `${name}: 异常, 已输出日志 ‼️`
      } else {
        merge[keyname].notify += `\n${name}: 异常, 已输出日志 ‼️ (2)`
      }
      merge[keyname].error = 1
    }
    return console.log(`\n‼️${name}发生错误\n‼️名称: ${er.name}\n‼️描述: ${er.message}${JSON.stringify(er).match(/\"line\"/)?`\n‼️行列: ${JSON.stringify(er)}`:``}${resp&&resp.status?`\n‼️状态: ${resp.status}`:``}${body?`\n‼️响应: ${resp&&resp.status!=503?body:`Omit.`}`:``}`)
  }
  const time = () => {
    const end = ((Date.now() - start) / 1000).toFixed(2)
    return console.log('\n签到用时: ' + end + ' 秒')
  }
  const done = (value = {}) => {
    if (isQuanX) return $done(value)
    if (isSurge) isRequest ? $done(value) : $done()
  }
  return {
    AnError,
    isRequest,
    isJSBox,
    isSurge,
    isQuanX,
    isLoon,
    isNode,
    notify,
    write,
    read,
    get,
    post,
    time,
    done
  }
};
ReadCookie();