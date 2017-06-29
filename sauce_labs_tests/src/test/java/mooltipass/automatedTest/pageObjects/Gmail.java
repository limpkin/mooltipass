package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;
public class Gmail  extends AbstractPage{
	
	public Gmail (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "identifierId")
	private WebElement email;

	@FindBy(xpath = "//input[@type='password']")
	private WebElement password;

	@FindBy(id = "identifierNext")
	private WebElement emailNextBtn;
	
	@FindBy(id = "passwordNext")
	private WebElement passwordNextBtn;
	
	
	@FindBy(id = "gb_71")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//a[@class='gb_b gb_db gb_R']")
	private WebElement expandBtn;


	public void enterEmail(String value){
		email.sendKeys(value);
	}
	
	public void enterPassword(String value){

		waitUntilAppears((By.xpath("//input[@type='password']")));
		password.sendKeys(value);
	}
	
	public void emailNextClick(){
		emailNextBtn.click();
	}
	
	public void passwordNextClick(){
		passwordNextBtn.click();
	}
	public void logout()
	{
		waitUntilAppears((By.xpath( "//a[@class='gb_b gb_db gb_R']")));
		expandBtn.click();
		waitUntilAppears((By.id("gb_71")));
		logoutBtn.click();
		
	}
	public boolean checkLogin(){
		//waitUntilAppears((By.xpath("//a[@href='https://accounts.google.com/SignOutOptions?hl=en&continue=https://mail.google.com/mail&service=mail']")));
		return isElementPresent(By.xpath("//a[@href='https://accounts.google.com/SignOutOptions?hl=en&continue=https://mail.google.com/mail&service=mail']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("identifierId"));
	}
}