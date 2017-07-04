package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Ricardo extends AbstractPage{

	public Ricardo(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//li/*[contains(text(),'Einloggen')]")
	private WebElement loginBtn;

	@FindBy(xpath = "//section[@id='it_LoginSection']//input[@type='text']")
	private WebElement email;

	@FindBy(xpath = "//input[@type='password']")
	private WebElement password;

	@FindBy(xpath = "//button[@id='modalsubmit']")
	private WebElement submitLogin;
	
	@FindBy(id = "hd-logout")
	private WebElement logoutBtn;
	
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void enterEmail(String value){

		if(waitUntilAppears(email))
				email.sendKeys(value);
		else
			driver.findElement(By.xpath("//input[@name='username']")).sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		if(waitUntilAppears(submitLogin))
			submitLogin.click();
		else
			driver.findElement(By.xpath("//input[@type='submit']")).click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.id("hd-logout"));
		return isElementPresent(By.id("hd-logout"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@type='password']"));
	}
	
	public void logout(){
		logoutBtn.click();

	}
}