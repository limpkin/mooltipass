package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class PcOstschweiz extends AbstractPage{

	public PcOstschweiz(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}



	@FindBy(xpath = "//input[@id='logNick']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='logPass']")
	private WebElement password;

	@FindBy(xpath = "//input[@id='accLogin']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//div[@class='logOut']/span")
	private WebElement logoutBtn;
	
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//div[@class='logOut']/span"));
		return isElementPresent(By.xpath("//div[@class='logOut']/span"));
	}


	public void logout(){
		logoutBtn.click();
		sleep(2000);
	}
}
